#include "HttpRequestAwait.h"
#include <jde/opc/UM.h>
#include <jde/opc/async/Write.h>
#include <jde/opc/async/SessionAwait.h>
#include <jde/opc/uatypes/Node.h>
#include <jde/opc/uatypes/UAClient.h>
#include <jde/opc/uatypes/Value.h>
#include <jde/opc/uatypes/UAException.h>

#define let const auto

namespace Jde::Opc{
	constexpr ELogTags _tags{ ELogTags::HttpServerRead };
	HttpRequestAwait::HttpRequestAwait( HttpRequest&& req, SL sl )ι:
		base{ move(req), sl }
	{}

	α HttpRequestAwait::await_ready()ι->bool{
		if( _request.IsGet("/ErrorCodes") ){
			vector<StatusCode> scs;
			string scsString = _request["scs"];
			auto strings = Str::Split( scsString );
			jarray j;
			for( let s : strings ){
				let sc = To<StatusCode>( s );
				j.push_back( {{"sc", sc},{"message", UAException::Message(sc)}} );
			}
			_readyResult = mu<jobject>( jobject{{"errorCodes", j}} );
		}
		return _readyResult!=nullptr;
	}
	α HttpRequestAwait::ParseNodes()ε->tuple<flat_set<NodeId>,jarray>{
		let& nodeJson = _request["nodes"];
		auto jNodes = Json::AsArray( Json::Parse(nodeJson) );
		flat_set<NodeId> nodes;
		for( let& node : jNodes )
			nodes.emplace( Json::AsObject(node) );
		if( nodes.empty() )
			throw RestException<http::status::bad_request>{ SRCE_CUR, move(_request), "empty nodes" };
		return make_tuple( nodes, move(jNodes) );
	}

	α HttpRequestAwait::ResumeSnapshots( flat_map<NodeId, Value>&& results, jarray&& j )ι->void{
		for( let& [nodeId, value] : results )
			j.push_back( jobject{{"node", nodeId.ToJson()}, {"value", value.ToJson()}} );
		Resume( {jobject{{"snapshots", j}}, move(_request)} );
	}

	α HttpRequestAwait::Browse()ι->Browse::ObjectsFolderAwait::Task{
		try{
			let snapshot = ToIV( _request["snapshot"] )=="true";
			_request.LogRead( Ƒ("BrowseObjectsFolder snapshot: {}", snapshot) );
			auto j = co_await Browse::ObjectsFolderAwait( NodeId{_request.Params()}, snapshot, move(_client) );
			Resume( {move(j), move(_request)} );
		}
		catch( IException& e ){
			ResumeExp( move(e) );
		}
	}

	α HttpRequestAwait::SnapshotWrite()ι->Jde::Task{
		try{
			let [nodes, jNodes] = ParseNodes();
			auto results = ( co_await Read::SendRequest(nodes, _client) ).UP<flat_map<NodeId, Value>>();
			if( find_if( *results, []( let& pair )->bool{ return pair.second.hasStatus && pair.second.status==UA_STATUSCODE_BADSESSIONIDINVALID; } )!=results->end() ) {
				co_await AwaitSessionActivation( _client );
				results = ( co_await Read::SendRequest(nodes, _client) ).UP<flat_map<NodeId, Value>>();
			}
			if( _request.Target()=="/Snapshot" )
				ResumeSnapshots( move(*results), jarray{} );
			else{ //target=="/Write"
				jarray jValues = Json::AsArray( Json::Parse(_request["values"]) );
				if( jNodes.size()!=jValues.size() )
					throw RestException<http::status::bad_request>{ SRCE_CUR, move(_request), "Invalid json: nodes.size={} values.size={}", nodes.size(), jValues.size() };
				flat_map<NodeId, Value> values;
				for( uint i=0; i<jNodes.size(); ++i ){
					NodeId node{  Json::AsObject(jNodes[i]) };
					if( auto existingValue = results->find(node); existingValue!=results->end() ){
						THROW_IF( existingValue->second.status, "Node {} has an error: {}.", serialize(node.ToJson()), UAException{existingValue->second.status}.ClientMessage() );
						existingValue->second.Set( jValues.at(i) );
						values.emplace( move(node), existingValue->second );
					}
					else
						throw RestException<http::status::bad_request>( SRCE_CUR, move(_request), "Node {} not found.", serialize(node.ToJson()) );
				}
				auto writeResults = ( co_await Write::SendRequest(move(values), _client) ).UP<flat_map<NodeId, UA_WriteResponse>>();
				flat_set<NodeId> successNodes;
				jarray array;
				for( auto& [nodeId, response] : *writeResults ){
					jarray j;
					bool error{};
					for( uint i=0; i<response.resultsSize;++i ){
						error = error || response.results[i];
						j.push_back( response.results[i] );
					}
					if( error )
						array.push_back( jobject{{"node", nodeId.ToJson()}, {"sc", j}} );
					else
						successNodes.insert( nodeId );
					UA_WriteResponse_clear( &response );
				}
				if( successNodes.empty() )
					Resume( {jobject{{"snapshots", array}}, move(_request)} );
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

	α HttpRequestAwait::CoHandleRequest()ι->ConnectAwait::Task{
		auto opcId = move( _request["opc"] );
		let& target = _request.Target();
		string userId, password;
		if( _request.SessionId() )
			tie( userId, password ) = Credentials( _request.SessionId(), opcId );
		try{
			_client = co_await UAClient::GetClient( move(opcId), userId, password );
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
			let body = _request.Body();
			let domain = Json::AsString( body, "opc" );
			let user = Json::AsString( body, "user" );
			let password = Json::AsString( body, "password" );
			_request.LogRead( Ƒ("Login - opc: {}, user: {}", domain, user) );
			let session = co_await AuthenticateAwait{ move(user), move(password), move(domain), endpoint, false };
			Resume( {jobject{{"sessionId", Ƒ("{:x}", session.session_id())}}, move(_request)} );
		}
		catch( IException& e ){
			ResumeExp( RestException<http::status::unauthorized>(move(e), move(_request)) );
		}
	}

	α HttpRequestAwait::Suspend()ι->void{
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
			: Promise()->Value() ? move( *Promise()->Value() ) : HttpTaskResult{ jobject{}, move(_request) };
	}
}