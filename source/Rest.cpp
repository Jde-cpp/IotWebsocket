#include "Rest.h"
#include <ranges>
#include "../../Framework/source/db/GraphQL.h"
#include "../../Framework/source/math/MathUtilities.h"
#include "async/Write.h"
#include "uatypes/Node.h"
#include "uatypes/UAClient.h"
#include "uatypes/helpers.h"


#define var const auto
namespace Jde::Iot
{
	sp<TListener<Session>> _listener{ ms<TListener<Session>>(Settings::Get<PortType>("rest/port").value_or(6707)) };
	α Rest::DoAccept()ι->void{
		IApplication::AddShutdown( _listener );
		_listener->DoAccept();
	}

	struct SessionInfoAwait final : IAwaitCache
	{
		SessionInfoAwait( SessionPK sessionId, SRCE )ι:IAwaitCache{sl}, _sessionId{sessionId}{}
		α await_ready()ι->bool override;
		α await_suspend( HCoroutine h )ι->void override;
	private:
		AwaitResult _result;
		const SessionPK _sessionId;
	};

	α SendSnapshots( flat_map<NodeId, Value>&& results, json j, Request&& req )ι->void{
		for( var& [nodeId, value] : results )
			j.push_back( json{{"node", nodeId.ToJson()}, {"value", value.ToJson()}} );
		Session::Send( json{{"snapshots", j}}, move(req) );
	}

	α CoHandleRequest( string target, flat_map<string,string> params, Request req )ι->Task{
		try{
			auto opcId = move( params["opc"] );
			auto pClient = (co_await UAClient::GetClient( move(opcId) )).SP<UAClient>();
	    if( req.Method() == http::verb::get ){
				if( target=="/BrowseObjectsFolder" ){
					var snapshot = ToIV( Find(params, "snapshot") )=="true";
					Browse::ObjectsFolder( move(pClient), NodeId{params}, move(req), snapshot );
				}
				else if( target=="/Snapshot" || target=="/Write" ){
					var nodeJson = Find(params, "nodes");
					json jNodes = json::parse( nodeJson );
					flat_set<NodeId> nodes;
					for( var& node : jNodes )
						nodes.emplace( node );
					if( nodes.empty() )
						co_return Session::Send( http::status::bad_request, "Invalid json: empty nodes", move(req) );
					auto results = ( co_await Read::SendRequest(nodes, pClient) ).UP<flat_map<NodeId, Value>>();
					if( find_if( *results, []( var& pair )->bool{ return pair.second.hasStatus && pair.second.status==UA_STATUSCODE_BADSESSIONIDINVALID; } )!=results->end() ){
						co_await AwaitSessionActivation( pClient );
						results = ( co_await Read::SendRequest(nodes, pClient) ).UP<flat_map<NodeId, Value>>();
					}

					if( target=="/Snapshot" )
						SendSnapshots( move(*results), json::array(), move(req) );
					else{
						json jValues = json::parse( Find(params, "values") );
						if( jNodes.size()!=jValues.size() )
							co_return Session::Send( http::status::bad_request, format("Invalid json: jNodes.size={} values.size={}", jNodes.size(), jValues.size()), move(req) );
						flat_map<NodeId, Value> values;
//						DBG( "jNodes={}", jNodes.dump() );
						for( uint i=0; i<jNodes.size(); ++i ){
							json jNode = jNodes[i];
							NodeId node{ jNode };
							if( auto existingValue = results->find(node); existingValue!=results->end() ){
								THROW_IF( existingValue->second.status, "Node {} has an error: {}.", node.ToJson().dump(), UAException{existingValue->second.status}.ClientMessage() );
								existingValue->second.Set( jValues.at(i) );
								values.emplace( move(node), existingValue->second );
							}
							else
								THROW( "Node {} not found.", node.ToJson().dump() );
						}
						auto results = ( co_await Write::SendRequest(move(values), pClient) ).UP<flat_map<NodeId, UA_WriteResponse>>();
						flat_set<NodeId> successNodes;
						json array = json::array();
						for( auto& [nodeId, response] : *results )
						{
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
							Session::Send( json{{"snapshots", array}}, move(req) );
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
							SendSnapshots( move(*updatedResults), move(array), move(req) );
						}
					}
				}
			}
			else if( req.Method() == http::verb::post )
				Session::Send( http::status::not_found, move(target), move(req) );
			else
	      Session::Send( http::status::forbidden, format("Only get/put verb is supported {}", (int)req.Method()), move(req) );
		}
		catch( const json::basic_json::type_error& e ){
			Session::Send( http::status::bad_request, format("Invalid json: {}", e.what()), move(req) );
		}
		catch( const json::exception& e ){
			Session::Send( http::status::bad_request, format("Invalid json: {}", e.what()), move(req) );
		}
		catch( const UAException& e ){
			Session::Send( http::status::internal_server_error, e.ClientMessage(), move(req) );
		}
		catch( const Exception& e ){
			Session::Send( http::status::internal_server_error, e.what(), move(req) );
		}
	}

	α Session::HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void{
		if( req.Method() == http::verb::get ){
			if( target=="/ErrorCodes" ){
				vector<StatusCode> scs;
				string scsString = Find( params, "scs" );
				auto strings = Str::Split( scsString );
				json j = json::array();
				for( var s : strings ){
					var sc = To<StatusCode>( s );
					j.push_back( {{"sc", sc},{"message", UAException::Message(sc)}} );
				}
				Session::Send( json{{"errorCodes", j}}, move(req) );
			}
		}
		if( req.Session && params.contains("opc") )
			CoHandleRequest( move(target), move(params), move(req) );
	}
}
