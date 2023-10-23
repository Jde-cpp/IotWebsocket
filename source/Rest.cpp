#include "Rest.h"
#include <ranges>
#include "../../Framework/source/db/GraphQL.h"
#include "../../Framework/source/math/MathUtilities.h"
#include "WebSession.h"
#include "uatypes/Node.h"
#include "uatypes/UAClient.h"
#include "uatypes/helpers.h"


#define var const auto
namespace Jde::Iot
{
	sp<TListener<Session>> _listener{ ms<TListener<Session>>(Settings::Get<PortType>("rest/port").value_or(6707)) };
	//α Listener()ι->TListener<Session>&{return *_listener;}
	α Rest::DoAccept()ι->void{ _listener->DoAccept(); }


	struct SessionInfoAwait final : IAwaitCache
	{
		SessionInfoAwait( SessionPK sessionId, SRCE )ι:IAwaitCache{sl}, _sessionId{sessionId}{}
		α await_ready()ι->bool override;
		α await_suspend( HCoroutine h )ι->void override;
	private:
		AwaitResult _result;
		const SessionPK _sessionId;
	};

	α CoHandleRequest( string target, flat_map<string,string> params, Request req )ι->Task{
		try{
			auto opcId = move( params["opc"] );
			auto pClient = (co_await UAClient::GetClient( move(opcId) )).SP<UAClient>();
	    if( req.Method() == http::verb::get ){
				if( target=="/BrowseObjectsFolder" ){
					var snapshot = ToIV( Find(params, "snapshot") )=="true";
					Browse::ObjectsFolder( move(pClient), NodeId{params}, move(req), snapshot );
				}
				else if( target=="/Snapshot" ){
					var nodeJson = Find(params, "nodes");
					json j;
					try{
						j = json::parse( nodeJson );
					}
					catch( json::exception& e ){
						co_return Session::Send( http::status::bad_request, format("Invalid json: {}", e.what()), move(req) );
					}
					flat_set<NodeId> nodes;
					for( var& node : j )
						nodes.emplace( node );
					auto results = ( co_await Read::SendRequest(move(nodes), move(pClient)) ).UP<flat_map<NodeId, Value>>();
					json array = json::array();
					for( var& [nodeId, value] : *results )
						array.push_back( json{{"node", nodeId.ToJson()}, {"value", value.ToJson()}} );
					Session::Send( json{{"snapshots", array}}, move(req) );
				}
			}
			else if( req.Method() == http::verb::post )
				Session::Send( http::status::not_found, move(target), move(req) );
			else
	      Session::Send( http::status::forbidden, format("Only get/put verb is supported {}", req.Method()), move(req) );
		}
		catch( const UAException& e ){
			Session::Send( http::status::internal_server_error, e.ClientMessage(), move(req) );
		}
		catch( const Exception& e ){
			Session::Send( http::status::internal_server_error, e.what(), move(req) );
		}
	}
	α Session::HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void{
		DBG( "params.size={}", params.size() );
		if( req.Method() == http::verb::get ){
			if( target=="/ErrorCodes" ){
				vector<UA_StatusCode> scs;
				string scsString = Find( params, "scs" );
				auto strings = Str::Split( scsString );
				json j = json::array();
				for( var s : strings ){
					var sc = To<UA_StatusCode>( s );
					j.push_back( {{"sc", sc},{"message", UAException::Message(sc)}} );
				}
				Session::Send( json{{"errorCodes", j}}, move(req) );
			}
		}
		if( req.Session )
			CoHandleRequest( move(target), move(params), move(req) );
	}
}
