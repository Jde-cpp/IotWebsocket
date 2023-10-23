#include "UAClient.h"

#include "Node.h"
#include "helpers.h"
#include "Value.h"
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
		if( sessionState == UA_SESSIONSTATE_ACTIVATED || connectStatus==UA_STATUSCODE_BADCONNECTIONREJECTED ){
			lg _{ _awaitingActivationMutex };
			if( auto p=find_if(_awaitingActivation, [ua](auto x){return ua==x.first->UAPointer();}); p!=_awaitingActivation.end() ){
				auto pClient = move(p->first);
				auto target = move(p->second);
				_awaitingActivation.erase( p );
				_awaitingActivationMutex.unlock();
				if( sessionState == UA_SESSIONSTATE_ACTIVATED ){
					{
						ul _{ _clientsMutex };
						ASSERT( _clients.find(target)==_clients.end() );
						_clients[target] = pClient;
					}
					ConnectAwait::Resume( move(pClient), move(target) );
				}
				else
					ConnectAwait::Resume( move(pClient), move(target), UAException{connectStatus} );
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
		//{ lg _{_requestMutex}; ASSERT( _requests.size() ); }
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
	α UAClient::Retry( function<void(sp<UAClient>&&, HCoroutine&&)> f, UAException e, sp<UAClient> pClient, HCoroutine h )ι->Task{
		//TODO limit retry attempts.
		str target = pClient->Target();
		{
			ul _{ _clientsMutex };
			if( auto p =_clients.find(target); p!=_clients.end() ){
				DBG( "[{:x}]({:x}) - {} - removing client.", pClient->Handle(), e.Code, e.what() );
				_clients.erase(p);
			}
			else
				DBG( "[{:x}]({:x}) - could not find client={:x}.", pClient->Handle(), e.Code, e.what() );
		}

		if( e.Code==UA_STATUSCODE_BADCONNECTIONCLOSED || e.Code==UA_STATUSCODE_BADSERVERNOTCONNECTED ){
			try{
				pClient = ( co_await GetClient(target) ).SP<UAClient>();
				f( move(pClient), move(h) );
			}
			catch( Exception& e ){
				ResumeEx( move(e), move(h) );
			}
		}
		else
			ResumeEx( move(e), move(h) );
	}
	α UAClient::SendBrowseRequest( Browse::Request&& request, sp<UAClient>&& pClient, HCoroutine&& h )ι->void{
		try{
			{
				lg _{ pClient->_requestMutex };
				UAε( UA_Client_sendAsyncBrowseRequest(pClient->_ptr, &request, Browse::OnResponse, nullptr, &pClient->RequestId) );
				pClient->_requests[pClient->RequestId] = move(h);
			}
			pClient->Process();
		}
		catch( UAException& e ){
			//Browse::Request b2{ move(request) };
			//SendBrowseRequest( move(request), move(pClient), move(h) );
			//var f = [r{move(request)}]( sp<UAClient>&& p, HCoroutine&& h )mutable{ var b2{move(r)}; };
			Retry( [r{move(request)}]( sp<UAClient>&& p, HCoroutine&& h )mutable{SendBrowseRequest( move(r), move(p), move(h) );}, move(e), move(pClient), move(h) );
		}
		DBG( "~Browse::Send" );
	}

	α UAClient::SendReadRequest( const flat_set<NodeId>&& nodeIds, sp<UAClient>&& pClient, HCoroutine&& h )ι->void{
		flat_map<UA_UInt32, NodeId> ids;
		Jde::Handle handle{};
		try{
			for( auto&& nodeId : nodeIds ){
				lg _{ pClient->_requestMutex };
				UAε( UA_Client_readValueAttribute_async(pClient->_ptr, nodeId.nodeId, Read::OnResponse, (void*)handle, &pClient->RequestId) );
				if( !handle )
					handle = pClient->RequestId;
				ids.emplace( pClient->RequestId, nodeId );
	 		}
			{
				lg _{ pClient->_requestMutex };
				pClient->_requests[handle] = move( h );
				pClient->_readRequests.try_emplace( handle, ReadRequest{move(ids), pClient} );
			}
			pClient->Process();
		}
		catch( UAException& e ){
			Retry( [n=move(nodeIds)]( sp<UAClient>&& p, HCoroutine&& h )mutable{SendReadRequest( move(n), move(p), move(h) );}, move(e), move(pClient), move(h) );
		}
	}
	α UAClient::SendWriteRequest( const flat_map<NodeId,Value>&& values, sp<UAClient>&& pClient, HCoroutine&& h )ι->void{
		flat_map<UA_UInt32, NodeId> ids;
		Jde::Handle handle{};
		try{
			for( auto&& [nodeId, value] : values ){
				lg _{ pClient->_requestMutex };
				myVariant
				UAε( UA_Client_writeValueAttribute_async(pClient->_ptr, nodeId.nodeId, &value.Variant(), Write::OnResonse, NULL,
                                                &reqId);

					UA_Client_readValueAttribute_async(, nodeId.nodeId, Read::OnResponse, (void*)handle, &pClient->RequestId) );
				if( !handle )
					handle = pClient->RequestId;
				ids.emplace( pClient->RequestId, nodeId );
	 		}
			{
				lg _{ pClient->_requestMutex };
				pClient->_requests[handle] = move( h );
				pClient->_readRequests.try_emplace( handle, ReadRequest{move(ids), pClient} );
			}
			pClient->Process();
		}
		catch( UAException& e ){
			Retry( [n=move(nodeIds)]( sp<UAClient>&& p, HCoroutine&& h )mutable{SendReadRequest( move(n), move(p), move(h) );}, move(e), move(pClient), move(h) );
		}
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
}