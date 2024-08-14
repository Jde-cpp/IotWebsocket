#include "HttpRequestAwait.h"
#include <jde/iot/UM.h>
#include <jde/iot/async/Write.h>
#include <jde/iot/uatypes/Node.h>
#include <jde/iot/uatypes/UAClient.h>
#include <jde/iot/uatypes/Value.h>
#define var const auto

namespace Jde::Iot{
	constexpr ELogTags _tags{ ELogTags::HttpServerRead };
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

	α HttpRequestAwait::ResumeSnapshots( flat_map<NodeId, Value>&& results, json&& j )ι->void{
		for( var& [nodeId, value] : results )
			j.push_back( json{{"node", nodeId.ToJson()}, {"value", value.ToJson()}} );
		Resume( {json{{"snapshots", j}}, move(_request)} );
	}

	α HttpRequestAwait::Browse()ι->Browse::ObjectsFolderAwait::Task{
		try{
			var snapshot = ToIV( _request["snapshot"] )=="true";
			_request.LogRead( 𐢜("BrowseObjectsFolder snapshot: {}", snapshot) );
			auto j = co_await Browse::ObjectsFolderAwait( NodeId{_request.Params()}, snapshot, move(_client) );
			Resume( {move(j), move(_request)} );
		}
		catch( IException& e ){
			ResumeExp( move(e) );
		}
	}

	α HttpRequestAwait::SnapshotWrite()ι->Jde::Task{
		try{
			var [nodes, jNodes] = ParseNodes();
			auto results = ( co_await Read::SendRequest(nodes, _client) ).UP<flat_map<NodeId, Value>>();
			if( find_if( *results, []( var& pair )->bool{ return pair.second.hasStatus && pair.second.status==UA_STATUSCODE_BADSESSIONIDINVALID; } )!=results->end() ) {
				co_await AwaitSessionActivation( _client );
				results = ( co_await Read::SendRequest(nodes, _client) ).UP<flat_map<NodeId, Value>>();
			}
			if( _request.Target()=="/Snapshot" )
				ResumeSnapshots( move(*results), json::array() );
			else{ //target=="/Write"
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
				auto writeResults = ( co_await Write::SendRequest(move(values), _client) ).UP<flat_map<NodeId, UA_WriteResponse>>();
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
					Resume( {json{{"snapshots", array}}, move(_request)} );
				else{
					up<flat_map<NodeId, Value>> updatedResults;
					try{
						updatedResults = ( co_await Read::SendRequest(move(successNodes), move(_client)) ).UP<flat_map<NodeId, Value>>();
					}
					catch( UAException& e ){
						if( !e.IsBadSession() )
							e.Throw();
					}
					if( !updatedResults ){
						co_await AwaitSessionActivation( _client );
						updatedResults = ( co_await Read::SendRequest(move(successNodes), move(_client)) ).UP<flat_map<NodeId, Value>>();
					}
					ResumeSnapshots( move(*updatedResults), move(array) );
				}
			}
		}
		catch( IException& e ){
			ResumeExp( move(e) );
		}
	}

	α HttpRequestAwait::CoHandleRequest()ι->Jde::Task{
		auto opcId = move( _request["opc"] );
		var& target = _request.Target();
		string userId, password;
		if( _request.SessionId() )
			tie( userId, password ) = Credentials( _request.SessionId(), opcId );
		try{
			_client = ( co_await UAClient::GetClient(move(opcId), userId, password) ).SP<UAClient>();
			if( _request.IsGet() ){
				if( target=="/BrowseObjectsFolder" )
					Browse();
				else if( target=="/Snapshot" || target=="/Write" )
					SnapshotWrite();
				else
					throw RestException<http::status::not_found>{ SRCE_CUR, move(_request), "Unknown target '{}'", _request.Target() };
			}
			else if( _request.IsPost() )
				throw RestException<http::status::not_found>{ SRCE_CUR, move(_request), "Post not supported for target '{}'", target };
			else
				throw RestException<http::status::forbidden>{ SRCE_CUR, move(_request), "Only get/post verb is supported for target '{}'", target };
		}
		catch( IException& e ){
			ResumeExp( move(e) );
		}
	}


	α HttpRequestAwait::Login( str endpoint )ι->AuthenticateAwait::Task{
		try{
			json body = _request.Body();
			var domain = Json::Get( body, "opc" );
			var user = Json::Get( body, "user" );
			var password = Json::Get( body, "password" );
			_request.LogRead( 𐢜("Login - opc: {}, user: {}", domain, user) );
			var session = co_await AuthenticateAwait{ user, password, domain, endpoint, false };
			Resume( {json{{"sessionId", 𐢜("{:x}", session.session_id())}}, move(_request)} );
		}
		catch( IException& e ){
			ResumeExp( RestException<http::status::unauthorized>(move(e), move(_request)) );
		}
	}

	α HttpRequestAwait::await_suspend( base::Handle h )ε->void{
		base::await_suspend(h);
		up<IException> pException;
 		if( _request.IsPost("/Login") )
			Login( _request.UserEndpoint.address().to_string() );
		else{
			if( _request.Contains("opc") )
				CoHandleRequest();
			else if( _request.Target().size() ){
				_request.LogRead();
				RestException<http::status::not_found> e{ SRCE_CUR, move(_request), "Unknown target '{}'", _request.Target() };
				ResumeExp( RestException<http::status::not_found>(move(e)) );
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