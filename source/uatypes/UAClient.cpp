#include "UAClient.h"

#include "Node.h"
#include "helpers.h"
#include "Value.h"
#include "Variant.h"
#include "../async/Attributes.h"
#include "../async/CreateSubscriptions.h"
#include "../async/DataChanges.h"
#include "../async/SetMonitoringMode.h"
#include "../async/Write.h"
#define var const auto

namespace Jde::Iot{
	flat_map<string,sp<UAClient>> _clients; shared_mutex _clientsMutex;
	flat_map<sp<UAClient>,string> _awaitingActivation; mutex _awaitingActivationMutex;

	UAClient::UAClient( str address )ε:
		UAClient{ OpcServer{address} }
	{}

	UAClient::UAClient(OpcServer&& opcServer)ε:
		_opcServer{ move(opcServer) },
		_ptr{ Create() },
		MonitoredNodes{ this }{
		UA_ClientConfig_setDefault( &Configuration() );
		DBG( "({:x})UAClient( '{}', '{}' )", Handle(), Target(), Url() );
//		DBG( "sizeof(UA_Client)={}", sizeof(UA_Client) );
	}
	UAClient::~UAClient(){
		_config.eventLoop->free( _config.eventLoop );
		UA_Client_delete( _ptr );
		DBG( "({:x})~UAClient( '{}', '{}' )", Handle(), Target(), Url() );
	}
	α UAClient::Shutdown()ι->void{
		for( var& [_,p] : _clients ){
			WARN_IF( p.use_count()>1, "({:x})use_count={}", p->Handle(), p.use_count() );
			p->MonitoredNodes.Shutdown();
		}
		ul _{ _clientsMutex };
		_clients.clear();
	}
	α stateCallback( UA_Client *ua, UA_SecureChannelState channelState, UA_SessionState sessionState, StatusCode connectStatus )ι->void{
		DBG( "({:x})channelState='{}', sessionState='{}', connectStatus='({:x}){}'", (uint)ua, UAException::Message(channelState), UAException::Message(sessionState), connectStatus, UAException::Message(connectStatus) );
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
	α subscriptionInactivityCallback( UA_Client *client, SubscriptionId subscriptionId, void *subContext ){
		DBG( "({:x}.{:x})subscriptionInactivityCallback", (uint)client, subscriptionId );
	}

	α UAClient::Create()ι->UA_Client*{
		_config.logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
		_config.eventLoop = UA_EventLoop_new_POSIX(&_config.logger);
		UA_ConnectionManager *tcpCM = UA_ConnectionManager_new_POSIX_TCP( "tcp connection manager"_uv );
		_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)tcpCM);
		_config.timeout = 10000; /*ms*/
		_config.stateCallback = stateCallback;
		_config.inactivityCallback = inactivityCallback;
		_config.subscriptionInactivityCallback = subscriptionInactivityCallback;
		UA_ConnectionManager *udpCM = UA_ConnectionManager_new_POSIX_UDP( "udp connection manager"_uv );
		_config.eventLoop->registerEventSource( _config.eventLoop, (UA_EventSource *)udpCM );
		auto ua = UA_Client_newWithConfig( &_config );
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
				DBG( "({:x})Removing client: ({:x}){}.", pClient->Handle(), e.Code, e.what() );
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
	α UAClient::SendBrowseRequest( Browse::Request&& request, HCoroutine&& h )ι->void{
		try{
			lg _{ _requestIdMutex };
			UAε( UA_Client_sendAsyncBrowseRequest(_ptr, &request, Browse::OnResponse, nullptr, &RequestIndex) );
			_requests.emplace( RequestIndex, mu<UARequest>(move(h)) );
		}
		catch( UAException& e ){
			//Browse::Request b2{ move(request) };
			//SendBrowseRequest( move(request), move(pClient), move(h) );
			//var f = [r{move(request)}]( sp<UAClient>&& p, HCoroutine&& h )mutable{ var b2{move(r)}; };
			Retry( [r{move(request)}]( sp<UAClient>&& p, HCoroutine&& h )mutable{p->SendBrowseRequest( move(r), move(h) );}, move(e), shared_from_this(), move(h) );
		}
		Process();
//		DBG( "~Browse::Send" );
	}

	α UAClient::SendReadRequest( const flat_set<NodeId>&& nodeIds, sp<UAClient>&& pClient, HCoroutine&& h )ι->void{
		flat_map<UA_UInt32, NodeId> ids;
		RequestId requestId{};
		try{
			for( auto&& nodeId : nodeIds ){
				lg _{ pClient->_requestIdMutex };
				UAε( UA_Client_readValueAttribute_async(pClient->_ptr, nodeId.nodeId, Read::OnResponse, (void*)(uint)requestId, &pClient->RequestIndex) );
				if( !requestId )
					requestId = pClient->RequestIndex;
				ids.emplace( pClient->RequestIndex, nodeId );
	 		}
			pClient->_requests.emplace( requestId, mu<UARequest>(move(h)) );
			pClient->_readRequests.try_emplace( requestId, UARequestMulti<Value>{move(ids), pClient} );
			pClient->Process();
		}
		catch( UAException& e ){
			Retry( [n=move(nodeIds)]( sp<UAClient>&& p, HCoroutine&& h )mutable{SendReadRequest( move(n), move(p), move(h) );}, move(e), move(pClient), move(h) );
		}
	}
	α UAClient::SendWriteRequest( flat_map<NodeId,Value>&& values, HCoroutine&& h )ι->void{
		flat_map<UA_UInt32, NodeId> ids;
		Jde::Handle requestId{};
		try{
			for( auto&& [nodeId, value] : values ){
				lg _{ _requestIdMutex };
				UAε( UA_Client_writeValueAttribute_async(_ptr, nodeId.nodeId, &value.value, Write::OnResonse, (void*)requestId, &RequestIndex) );
				if( !requestId )
					requestId = RequestIndex;
				ids.emplace( RequestIndex, nodeId );
	 		}
			_requests.emplace( requestId, mu<UARequest>(move(h)) );
			_writeRequests.try_emplace( requestId, UARequestMulti<UA_WriteResponse>{move(ids), shared_from_this()} );
			Process();
		}
		catch( UAException& e ){
			Retry( [n=move(values)]( sp<UAClient>&& p, HCoroutine&& h )mutable{p->SendWriteRequest( move(n), move(h) );}, move(e), shared_from_this(), move(h) );
		}
	}
	α UAClient::SetMonitoringMode( Iot::SubscriptionId subscriptionId )ι->void{
		UA_SetMonitoringModeRequest request{};
		request.subscriptionId = subscriptionId;
		try{
			lg _{ _requestIdMutex };
			UAε( UA_Client_MonitoredItems_setMonitoringMode_async(UAPointer(), move(request), SetMonitoringModeCallback, nullptr, &RequestIndex) );
			_requests.emplace( RequestIndex, nullptr );
		}
		catch( UAException& e ){
			Retry( [subscriptionId]( sp<UAClient>&& p, HCoroutine&& h )mutable{p->SetMonitoringMode( subscriptionId );}, move(e), shared_from_this(), {} );
		}
		Process();
	}

	α UAClient::CreateSubscriptions()ι->void{
		try{
			lg _{ _requestIdMutex };
			UAε( UA_Client_Subscriptions_create_async(UAPointer(), UA_CreateSubscriptionRequest_default(), nullptr, StatusChangeNotificationCallback, DeleteSubscriptionCallback, CreateSubscriptionCallback, nullptr, &RequestIndex) );
			_requests.emplace( RequestIndex, nullptr );
		}
		catch( UAException& e ){
			Retry( []( sp<UAClient>&& p, HCoroutine&& h )mutable{p->CreateSubscriptions();}, move(e), shared_from_this(), {} );
		}
		Process();
	}

	α UAClient::DataSubscriptions( CreateMonitoredItemsRequest&& request, Jde::Handle requestHandle, HCoroutine&& h )ι->void{
		try{
			UA_Client_DataChangeNotificationCallback notifications = DataChangesCallback;
			UA_Client_DeleteMonitoredItemCallback deleteCallbacks = DataChangesDeleteCallback;
			void* contexts = nullptr;
			ASSERT(CreatedSubscriptionResponse);
			if( !CreatedSubscriptionResponse )
				return ResumeEx( Exception{"CreatedSubscriptionResponse==null"}, move(h) );
			request.subscriptionId = CreatedSubscriptionResponse->subscriptionId;
			lg _{ _requestIdMutex };
			UAε( UA_Client_MonitoredItems_createDataChanges_async(UAPointer(), request, &contexts, &notifications, &deleteCallbacks, CreateDataChangesCallback, (void*)requestHandle, &RequestIndex) );
			_requests.emplace( RequestIndex, mu<UARequest>(move(h)) );
		}
		catch( UAException& e ){
			Retry( [&x=request,requestHandle]( sp<UAClient>&& p, HCoroutine&& h )mutable{p->DataSubscriptions(move(x), requestHandle, move(h));}, move(e), shared_from_this(), move(h) );
		}
		Process();
	}

	α UAClient::DataSubscriptionDelete( Iot::SubscriptionId subscriptionId, flat_set<MonitorId>&& monitoredItemIds )ι->void{
		UA_DeleteMonitoredItemsRequest request; UA_DeleteMonitoredItemsRequest_init(&request);
		request.subscriptionId = subscriptionId;
		request.monitoredItemIdsSize = monitoredItemIds.size();
		request.monitoredItemIds = (UA_UInt32*)UA_Array_new( monitoredItemIds.size(), &UA_TYPES[UA_TYPES_UINT32] );
		uint i=0;
		for( auto& id : monitoredItemIds )
			request.monitoredItemIds[i++] = id;
		try{
			{
				lg lock{ _requestIdMutex };
				UAε( UA_Client_MonitoredItems_delete_async(UAPointer(), move(request), MonitoredItemsDeleteCallback, nullptr, &RequestIndex) );
				_requests.emplace( RequestIndex, mu<UARequest>(nullptr) );
			}
			Process();
			UA_DeleteMonitoredItemsRequest_deleteMembers( &request );
		}
		catch( const Exception&)
		{}
	}

	α UAClient::RequestDataTypeAttributes( const flat_set<NodeId>&& x, HCoroutine&& h )ι->void
	{
		flat_map<UA_UInt32, NodeId> ids;
		Jde::Handle requestId{};
		try{
			for( var& node : x ){
				lg _{ _requestIdMutex };
				UAε( UA_Client_readDataTypeAttribute_async(_ptr, node.nodeId, Attributes::OnResonse, (void*)requestId, &RequestIndex) );
				if( !requestId )
					requestId = RequestIndex;
				ids.emplace( RequestIndex, node );
	 		}
			_requests.emplace( requestId, mu<UARequest>(move(h)) );
			_dataAttributeRequests.try_emplace( requestId, UARequestMulti<NodeId>{move(ids), shared_from_this()} );
			Process();
		}
		catch( UAException& e ){
			Retry( [n=move(x)]( sp<UAClient>&& p, HCoroutine&& h )mutable{p->RequestDataTypeAttributes( move(n), move(h) );}, move(e), shared_from_this(), move(h) );
		}
	}

	α UAClient::Unsubscribe( const sp<IDataChange>&& dataChange )ι->void{
		sl _{ _clientsMutex };
		for( auto& [_, pClient] : _clients )
			pClient->MonitoredNodes.Unsubscribe( dataChange );
	}

	α UAClient::Find( UA_Client* ua, SL srce )ε->sp<UAClient>{
		sl _{ _clientsMutex };
		sp<UAClient> y;
		if( auto p = find_if( _clients, [ua](var& c){return c.second->_ptr==ua;} ); p!=_clients.end() )
			y = p->second;
		else
		 	throw Exception{ srce, ELogLevel::Critical, "({})Could not find client.", format("{:x}", (uint)ua) };

		return y;
	}

	α UAClient::TryFind( UA_Client* ua, SL sl )ι->sp<UAClient>{
		sp<UAClient> p;
		try{
			p = Find( ua, sl );
		}
		catch( const Exception& )
		{}
		return p;
	}

	α UAClient::Find( str id )ι->sp<UAClient>{
		sl _{ _clientsMutex };
		sp<UAClient> y;
		if( auto p = id.size() ? _clients.find(id) : find_if( _clients, [](var& c){return c.second->IsDefault();} ); p!=_clients.end() )
			y = p->second;
		return y;
	}
}