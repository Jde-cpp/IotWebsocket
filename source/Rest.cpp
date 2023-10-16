#include "Rest.h"
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
			}
			else if( req.Method() == http::verb::post )
				Session::Send( http::status::not_found, move(target), move(req) );
			else
	      Session::Send( http::status::forbidden, format("Only get/put verb is supported {}", req.Method()), move(req) );
		}
		catch( const Exception& e )
		{
			Session::Send( http::status::internal_server_error, e.what(), move(req) );
		}
	}
	α Session::HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void{
		CoHandleRequest( move(target), move(params), move(req) );
	}
}
