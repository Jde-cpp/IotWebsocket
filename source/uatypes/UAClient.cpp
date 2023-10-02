#include "UAClient.h"
#include "UAException.h"
#include "helpers.h"

#define var const auto
namespace Jde::Iot
{
	flat_map<string,sp<UAClient>> _clients; shared_mutex _clientsMutex;
	flat_map<sp<UAClient>,string> _awaitingActivation; mutex _awaitingActivationMutex;

	UAClient::UAClient( str address )ε:
		UAClient{ OpcServer{address} }
	{}

	UAClient::UAClient(OpcServer&& opcServer)ε:
		_opcServer{ move(opcServer) },
		_ptr{ Create() }
	{
		UA_ClientConfig_setDefault( &Configuration() );
		//_ptr->log();
		//Connect();
		//log();
	}

	α stateCallback( UA_Client *ua, UA_SecureChannelState channelState, UA_SessionState sessionState, UA_StatusCode connectStatus )ι->void{
		DBG( "({:x})channelState='{}', sessionState='{}', connectStatus='({:x}){}", (uint)ua, UAException::Message(channelState), UAException::Message(sessionState), connectStatus, UAException::Message(connectStatus) );
		if( sessionState == UA_SESSIONSTATE_ACTIVATED ){
			lg _{ _awaitingActivationMutex };
			if( auto p=find_if(_awaitingActivation, [ua](auto x){return ua==x.first->UAPointer();}); p!=_awaitingActivation.end() ){
				auto pClient = move(p->first);
				auto id = move(p->second);
				_awaitingActivation.erase( p );
				_awaitingActivationMutex.unlock();
				{
					ul _{ _clientsMutex };
					ASSERT( _clients.find(id)==_clients.end() );
					_clients[id] = pClient;
				}
				ConnectAwait::Resume( move(pClient), move(id) );//another thread?
			}
		}
	}
	α inactivityCallback(UA_Client *client)->void{
		BREAK;
	}
	α subscriptionInactivityCallback( UA_Client *client, UA_UInt32 subscriptionId, void *subContext ){
		BREAK;
	}

	α UAClient::Create()ι->UA_Client*
	{
		_config.logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
		_config.eventLoop = UA_EventLoop_new_POSIX(&_config.logger);
		UA_ConnectionManager *tcpCM = UA_ConnectionManager_new_POSIX_TCP( "tcp connection manager"_uv );
		_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)tcpCM);
		_config.timeout = 10000; /*ms*/
		_config.stateCallback = stateCallback;
		_config.inactivityCallback = inactivityCallback;
		_config.subscriptionInactivityCallback = subscriptionInactivityCallback;
		UA_ConnectionManager *udpCM = UA_ConnectionManager_new_POSIX_UDP( "udp connection manager"_uv );
		_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)udpCM);
		auto ua = UA_Client_newWithConfig(&_config);
		UA_Client_getConfig(ua)->eventLoop->logger = &_config.logger;

		return ua;
	}
	α UAClient::OnSessionActivated( sp<UAClient> pClient, string id )ι->void{
		lg _{ _awaitingActivationMutex };
		ASSERT( _awaitingActivation.find(pClient)==_awaitingActivation.end() );
		_awaitingActivation[pClient] = id;
	}
	α UAClient::Process()ι->void{
		lg _{ _asyncRequestMutex };
		if( !_asyncRequest )
		{
			auto p = _asyncRequest = ms<AsyncRequest>( shared_from_this() );
			_asyncRequestMutex.unlock();
			p->Process();
		}
	}
	// α UAClient::Connect()ε->void
	// {
	// 	var r = UA_Client_connect( _ptr, _pOpcServer->Url.c_str() ); THROW_IFX( r!= UA_STATUSCODE_GOOD, UAException{ r } );
	// }

	α fileBrowsed( UA_Client *ua, void* userdata, UA_UInt32 requestId, UA_BrowseResponse* response )ι->void{
		DBG( "fileBrowsed={:x}.{}", (uint)ua, requestId );
		optional<HCoroutine> h = UAClient::ClearRequest( ua, requestId ); if( !h ) return;

		if( response->responseHeader.serviceResult )
			return ResumeEx( move(*h), UAException{response->responseHeader.serviceResult} );

		json references{ json::array() };
    for(size_t i = 0; i < response->resultsSize; ++i) {
      for(size_t j = 0; j < response->results[i].referencesSize; ++j) {
        const UA_ReferenceDescription& ref = response->results[i].references[j];
				json reference;
				reference["referenceType"] = ToJson( ref.referenceTypeId );
				reference["isForward"] = ref.isForward;
				reference["node"] = ToJson( ref.nodeId );

				json bn;
				const UA_QualifiedName& browseName = ref.browseName;
				bn["nsIndex"] = browseName.namespaceIndex;
				bn["name"] = ToSV( browseName.name );
				reference["browseName"] = bn;

				json dn;
				const UA_LocalizedText& displayName = ref.displayName;
				dn["locale"] = ToSV( displayName.locale );
				dn["text"] = ToSV( displayName.text );
				reference["displayName"] = dn;

				reference["nodeClass"] = ref.nodeClass;
				reference["typeDefinition"] = ToJson( ref.typeDefinition );

				references.push_back( reference );
			}
		}
		json j;
		j["references"] = references;
		h->promise().get_return_object().SetResult( mu<json>(move(j)) );
		DBG( "~fileBrowsed={:x}", (uint)userdata );
		h->resume();
	}

	α UAClient::SendBrowseRequest( UA_BrowseRequest bReq, HCoroutine h )ι->Task{
		UA_StatusCode sc; bool resent{};
		{
			lg _{ _requestMutex };
			sc = UA_Client_sendAsyncBrowseRequest( _ptr, &bReq, fileBrowsed, nullptr, &RequestId );
			if( sc ){
				_requestMutex.unlock();
				{
					ul _{ _clientsMutex };
					if( auto p =_clients.find(Target()); p!=_clients.end() ){
						DBG( "({:x}){} - removing client.", sc, UAException::Message(sc) );
						_clients.erase(p);
					}
					else
						DBG( "({:x}){} - could not find client.", sc, UAException::Message(sc) );
				}
				if( sc==UA_STATUSCODE_BADCONNECTIONCLOSED ){
					try{
						auto pClient = ( co_await GetClient(Target()) ).SP<UAClient>();
						pClient->SendBrowseRequest( move(bReq), move(h) );
						resent = true;
					}
					catch( Exception& e ){
						ResumeEx( move(h), move(e) );
					}
				}
				else
					ResumeEx( move(h), UAException{sc} );
			}
			else
				_requests[RequestId] = move(h);
		}
		if( !resent )
			UA_BrowseRequest_clear( &bReq );
		if( !sc )
			Process();
		DBG( "~SendBrowseRequest" );
	}

	α UAClient::ClearRequest( UA_Client* ua, UA_UInt32 requestId )ι->optional<HCoroutine>{
		optional<HCoroutine> h;
		try{
		 	auto p = Find( ua );

		 	lg l{ p->_requestMutex };
		 	auto handle = p->_requests.find( requestId ); if( handle==p->_requests.end() ) throw Exception{ SRCE_CUR, ELogLevel::Critical, "Could not find handle for client={}, request={}.", p->Target(), requestId };
			h = handle->second;
			p->_requests.erase( requestId );
			if( p->_requests.empty() )
			{
				lg __{ p->_asyncRequestMutex };
				p->_asyncRequest = nullptr;
			}
		}
		catch( Exception& e ){}
		return h;
	}

	// α ClientAwaitable::await_ready()ι->bool
	// {
	// 	sl _{ _clientsMutex };
	// 	if( auto p = _id.size() ? _clients.find(_id) : find_if( _clients, [](var& c){return c.second->IsDefault();} ); p!=_clients.end() )
	// 		_result.Set( p->second );
	// 	return _result.HasShared();
	// }

	α UAClient::Find( UA_Client* ua )ε->sp<UAClient>{
		sl _{ _clientsMutex };
		sp<UAClient> y;
		if( auto p = find_if( _clients, [ua](var& c){return c.second->_ptr==ua;} ); p!=_clients.end() )
			y = p->second;
		else
		 	throw Exception{ SRCE_CUR, ELogLevel::Critical, "Could not find client {:x}.", ua };
		return y;
	}
	α UAClient::Find( str id )ι->sp<UAClient>{
		ul _{ _clientsMutex };
		sp<UAClient> y;
		if( auto p = id.size() ? _clients.find(id) : find_if( _clients, [](var& c){return c.second->IsDefault();} ); p!=_clients.end() )
			y = p->second;
		return y;
	}

/*	α GetClient( string id, HCoroutine h )ι->Task{
		try{
			auto pServer = ( co_await OpcServer::Select(id) ).UP<OpcServer>(); THROW_IF( !pServer, "Could not find opc server:  '{}'", id );
			sp<UAClient> y = UAClient::Find( id );
			if( !y ){
				y = ms<UAClient>( move(pServer) );
				DBG( "Adding client ({:x}){}", y->Handle(), y->Target() );
				ul _{ _clientsMutex };
				_clients.emplace( y->Target(), y );
			}
			Result(h).SetSP( move(y) );
		}
		catch( Exception& e ){
			Result(h).SetResult( move(e) );
		}
		h.resume();
	}
*/
	// α ClientAwaitable::await_suspend( HCoroutine h )ι->void
	// {
	// 	IAwaitCache::await_suspend( h );
	// 	GetClient( _id, h );
	// }

}