#include "HttpRequestAwait.h"
#include <jde/iot/UM.h>
#include <jde/iot/async/Write.h>
#include <jde/iot/uatypes/Node.h>
#include <jde/iot/uatypes/UAClient.h>
#include <jde/iot/uatypes/Value.h>
#define var const auto

namespace Jde::Iot{
	HttpRequestAwait::HttpRequestAwait( HttpRequest&& req, SL sl )ι:
		base{ move(req), sl }
	{}

	α HttpRequestAwait::await_ready()ι->bool{
		optional<json> result;
		if( _request.IsGet("/ErrorCodes") ){
			vector<StatusCode> scs;
			string scsString = _request["scs"];
			auto strings = Str::Split( scsString );
			json j = json::array();
			for( var s : strings ){
				var sc = To<StatusCode>( s );
				j.push_back( {{"sc", sc},{"message", UAException::Message(sc)}} );
			}
			_readyResult = mu<json>( json{{"errorCodes", j}} );//are brackets right?
		}
		return _readyResult!=nullptr;
	}
	α HttpRequestAwait::ParseNodes()ε->tuple<flat_set<NodeId>,json>{
		var& nodeJson = _request["nodes"];
		json jNodes = Json::Parse( nodeJson );
		flat_set<NodeId> nodes;
		for( var& node : jNodes )
			nodes.emplace( node );
		if( nodes.empty() )
			throw RestException<http::status::bad_request>{ SRCE_CUR, move(_request), "empty nodes" };
		return make_tuple( nodes, jNodes );
	}

	α HttpRequestAwait::SetSnapshotResult( flat_map<NodeId, Value>&& results, json&& j )ι->void{
		for( var& [nodeId, value] : results )
			j.push_back( json{{"node", nodeId.ToJson()}, {"value", value.ToJson()}} );
		Promise()->SetValue( {json{{"snapshots", j}}, move(_request)} );
	}

	α HttpRequestAwait::CoHandleRequest()ι->Jde::Task{
		try{
			auto opcId = move( _request["opc"] );
			string userId, password;
/*		var login{ req.Method() == http::verb::put && target=="/Login" };
			if( login ){
				userId = Find( params, "user" );
				password = Find( params, "password" );
				if( userId.empty() )
					co_return Session::Send( http::status::bad_request, "Invalid json: empty user", move(req) );
			}
			else*/
			if( _request.SessionId() )
				tie( userId, password ) = Credentials( _request.SessionId(), opcId );

			auto pClient = ( co_await UAClient::GetClient(move(opcId), userId, password) ).SP<UAClient>();
/*			if( login ){
				auto sessionId = *( co_await Authenticate(userId, password, opcId) ).UP<SessionPK>();
				Session::Send( json{{"value", sessionId}}, move(req) );
			}
	    else*/
			var& target = _request.Target();
			if( _request.IsGet() ){
				if( target=="/BrowseObjectsFolder" ){
					var snapshot = ToIV( _request["snapshot"] )=="true";
					[&]()->Browse::ObjectsFolderAwait::Task{
						auto j = co_await Browse::ObjectsFolderAwait( NodeId{_request.Params()}, snapshot, move(pClient) );
						Promise()->SetValue( {move(j), move(_request)} );
					}();
				}
				else if( target=="/Snapshot" || target=="/Write" ){
					var [nodes, jNodes] = ParseNodes();
					auto results = ( co_await Read::SendRequest(nodes, pClient) ).UP<flat_map<NodeId, Value>>();
					if( find_if( *results, []( var& pair )->bool{ return pair.second.hasStatus && pair.second.status==UA_STATUSCODE_BADSESSIONIDINVALID; } )!=results->end() ) {
						co_await AwaitSessionActivation( pClient );
						results = ( co_await Read::SendRequest(nodes, pClient) ).UP<flat_map<NodeId, Value>>();
					}
					if( target=="/Snapshot" )
						SetSnapshotResult( move(*results), json::array() );
					else{
						json jValues = Json::Parse( _request["values"] );
						if( jNodes.size()!=jValues.size() )
							throw RestException<http::status::bad_request>{ SRCE_CUR, move(_request), "Invalid json: nodes.size={} values.size={}", nodes.size(), jValues.size() };
						flat_map<NodeId, Value> values;
						for( uint i=0; i<jNodes.size(); ++i ){
							NodeId node{  jNodes[i] };
							if( auto existingValue = results->find(node); existingValue!=results->end() ){
								THROW_IF( existingValue->second.status, "Node {} has an error: {}.", node.ToJson().dump(), UAException{existingValue->second.status}.ClientMessage() );
								existingValue->second.Set( jValues.at(i) );
								values.emplace( move(node), existingValue->second );
							}
							else
								throw RestException<http::status::bad_request>( SRCE_CUR, move(_request), "Node {} not found.", node.ToJson().dump() );
						}
						auto writeResults = ( co_await Write::SendRequest(move(values), pClient) ).UP<flat_map<NodeId, UA_WriteResponse>>();
						flat_set<NodeId> successNodes;
						json array = json::array();
						for( auto& [nodeId, response] : *writeResults ){
							json j = json::array();
							bool error{};
							for( uint i=0; i<response.resultsSize;++i ){
								error = error || response.results[i];
								j.push_back( response.results[i] );
							}
							if( error )
								array.push_back( json{{"node", nodeId.ToJson()}, {"sc", j}} );
							else
								successNodes.insert( nodeId );
							UA_WriteResponse_clear( &response );
						}
						if( successNodes.empty() )
							Promise()->SetValue( {json{{"snapshots", array}}, move(_request)} );
						else{
							up<flat_map<NodeId, Value>> updatedResults;
							try{
								updatedResults = ( co_await Read::SendRequest(move(successNodes), move(pClient)) ).UP<flat_map<NodeId, Value>>();
							}
							catch( UAException& e ){
								if( !e.IsBadSession() )
									e.Throw();
							}
							if( !updatedResults ){
								co_await AwaitSessionActivation( pClient );
								updatedResults = ( co_await Read::SendRequest(move(successNodes), move(pClient)) ).UP<flat_map<NodeId, Value>>();
							}
							SetSnapshotResult( move(*updatedResults), move(array) );
						}
					}
				}
				else
					throw RestException<http::status::not_found>{ SRCE_CUR, move(_request), "Unknown target '{}'", _request.Target() };
			}
			else if( _request.IsPost() )
				throw RestException<http::status::not_found>{ SRCE_CUR, move(_request), "Post not supported for target '{}'", target };
			else
				throw RestException<http::status::forbidden>{ SRCE_CUR, move(_request), "Only get/post verb is supported for target '{}'", target };
		}
		catch( UAException& e ){
			Promise()->SetError( move(e) );
		}
		catch( IException& e ){
			Promise()->SetError( move(e) );
		}
	}


	α HttpRequestAwait::Login()ι->Jde::Task{
		try{
			json body = _request.Body();
			var domain = Json::Get( body, "opc" );
			var user = Json::Get( body, "user" );
			var password = Json::Get( body, "password" );
			var sessionId = *( co_await Authenticate(user, password, domain) ).UP<SessionPK>();
			Promise()->SetValue( {json{{"sessionId", sessionId}}, move(_request)} );//are brackets right?
		}
		catch( UAException& e ){
			Promise()->SetError( move(e) );
		}
		catch( Exception& e ){
			Promise()->SetError( move(e) );
		}
	}

	α HttpRequestAwait::await_suspend( base::Handle h )ε->void{
		base::await_suspend(h);
		up<IException> pException;
 		if( _request.IsPost("/Login") )
			Login();
		else{
			if( _request.Contains("opc") )
				CoHandleRequest();
			else if( _request.Target().size() ){
				_request.LogRead();
				RestException<http::status::not_found> e{ SRCE_CUR, move(_request), "Unknown target '{}'", _request.Target() };
				ResumeEx( RestException<http::status::not_found>(move(e)) );
			}
		}
	}

	α HttpRequestAwait::await_resume()ε->HttpTaskResult{
		if( auto e = Promise() ? Promise()->MoveError() : nullptr; e ){
			auto pRest = dynamic_cast<IRestException*>( e.get() );
			if( pRest )
				pRest->Throw();
			else
				throw RestException<>{ move(*e), move(_request) };
		}
		return _readyResult
			? HttpTaskResult{ move(*_readyResult), move(_request) }
			: Promise()->Value() ? move( *Promise()->Value() ) : HttpTaskResult{ json{}, move(_request) };
	}
}