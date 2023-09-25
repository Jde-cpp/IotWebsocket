#include "Rest.h"
#include "../../Framework/source/db/GraphQL.h"
#include "../../Framework/source/math/MathUtilities.h"
#include "WebSession.h"
#include "uatypes/UAClient.h"


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


	struct BrowseFoldersAwait final : IAwait
	{
		BrowseFoldersAwait( sp<UAClient>&& c, SRCE )ι:IAwait{sl},_client{move(c)}{}
		α await_suspend( HCoroutine h )ι->void override;
		α await_resume()ι->AwaitResult override{ return _pPromise->get_return_object().Result(); }
	private:
		sp<UAClient> _client;
		sp<AsyncRequest> _processor;
	};

	static void fileBrowsed( UA_Client *client, void *userdata, UA_UInt32 requestId, UA_BrowseResponse *response )
	{
    // printf("%-50s%u\n", "Received BrowseResponse for request ", requestId);
    // UA_String us = *(UA_String *) userdata;
    // printf("---%.*s passed safely \n", (int) us.length, us.data);
		HCoroutine h = *(HCoroutine*)userdata;
		h.resume();

	}

	α BrowseFoldersAwait::await_suspend( HCoroutine h )ι->void
	{
		UA_SessionState ss;
		UA_Client_getState( *_client, nullptr, &ss, nullptr );
		ASSERT(ss == UA_SESSIONSTATE_ACTIVATED);

		IAwait::await_suspend( h );
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init( &bReq );
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;
		lock_guard _{ _client->RequestIdMutex };
		UA_Client_sendAsyncBrowseRequest( *_client, &bReq, fileBrowsed, &h, &_client->RequestId );
		_processor = UAClient::GetAsyncRequest( move(_client) );
		//start session, process messages.
	}
	α BrowseFolders( sp<UAClient>&& c )ι->BrowseFoldersAwait{ return BrowseFoldersAwait{move(c)}; }

	α BrowseObjectsFolder( string&& opcId, Request req )->Task
	{
		auto c = ( co_await UAClient::GetClient(move(opcId)) ).SP<UAClient>();
		auto y = ( co_await BrowseFolders(move(c)) ).UP<json>();
		Session::Send( move(*y), move(req) );
	}

	α Session::HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void
	{
		try
		{
			auto opcId = move( params["opc"] );
	    if( req.Method() == http::verb::get )
			{
				if( target=="/BrowseObjectsFolder" )
					BrowseObjectsFolder( move(opcId), move(req) );
			}
			else if( req.Method() == http::verb::post )
			{
				// if( target=="/GoogleLogin" )
				// {
				// 	GoogleLogin( move(req), move(s) );
				// 	//s->Send( result, move(req) );
				// }
				//else
					Send( http::status::not_found, move(target), move(req) );
			}
			else if( req.Method() == boost::beast::http::verb::options )
				SendOptions( move(req) );
			else
	      Send( http::status::forbidden, format("Only get/put verb is supported {}", req.Method()), move(req) );
		}
		catch( const Exception& e )
		{
			Send( http::status::internal_server_error, e.what(), move(req) );
		}
	}

}
