#include "Rest.h"
#include "../../Framework/source/db/GraphQL.h"
#include "../../Framework/source/math/MathUtilities.h"
#include "WebSession.h"
#include "uatypes/Node.h"
#include "uatypes/UAClient.h"
#include "uatypes/UAException.h"
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

	struct BrowseFoldersAwait final : IAwait
	{
		BrowseFoldersAwait( sp<UAClient>&& c, NodeId&& node,  SRCE )ι:IAwait{sl},_client{move(c)}, _node{move(node)}{}
		α await_suspend( HCoroutine h )ι->void override;
		α await_resume()ι->AwaitResult override{ return _pPromise->get_return_object().Result(); }
	private:
		sp<UAClient> _client;
		NodeId _node;
	};

	α BrowseFoldersAwait::await_suspend( HCoroutine h )ι->void{
		IAwait::await_suspend( h );
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init( &bReq );
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = _node.Move();
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;

		_client->SendBrowseRequest( move(bReq), move(h) );
		DBG("~BrowseFoldersAwait::await_suspend");
	}
	α BrowseFolders( sp<UAClient>&& c, NodeId&& node )ι->BrowseFoldersAwait{ return BrowseFoldersAwait{ move(c), move(node) }; }

	α BrowseObjectsFolder( string&& opcId, NodeId node, Request req )ι->Task
	{
		try{
			auto c = ( co_await UAClient::GetClient(move(opcId)) ).SP<UAClient>();
			auto y = ( co_await BrowseFolders(move(c), move(node)) ).UP<json>();
			Session::Send( move(*y), move(req) );
		}
		catch( Exception& e ){
			Session::Send( move(e), move(req) );
		}

		DBG("~BrowseObjectsFolder");
	}

	α Session::HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void
	{
		try
		{
			auto opcId = move( params["opc"] );
	    if( req.Method() == http::verb::get )
			{
				if( target=="/BrowseObjectsFolder" )
					BrowseObjectsFolder( move(opcId), NodeId(params), move(req) );
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
