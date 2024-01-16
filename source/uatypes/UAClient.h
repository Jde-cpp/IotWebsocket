#pragma once
#include "Browse.h"
#include "Logger.h"
#include "CreateMonitoredItemsRequest.h"
#include "../async/AsyncRequest.h"
#include "../async/ConnectAwait.h"
#include "../types/OpcServer.h"
#include "../types/MonitoringNodes.h"
#include "helpers.h"

namespace Jde::Iot{
	namespace Read{ α OnResponse( UA_Client *client, void *userdata, RequestId requestId, StatusCode status, UA_DataValue *var )ι->void; }
	namespace Write{ α OnResonse( UA_Client *ua, void *userdata, RequestId requestId, UA_WriteResponse *response )ι->void; }
	namespace Attributes{α OnResonse( UA_Client* ua, void* userdata, RequestId requestId, StatusCode status, UA_NodeId* dataType )ι->void;}

	struct UAClient;
	struct Value;
//	using CreateMonitoredItemsRequestPtr = UAUP<UA_CreateMonitoredItemsRequest, UA_CreateMonitoredItemsRequest_delete>;
	struct UARequest{
		UARequest( HCoroutine&& h )ι:CoHandle{ move(h) }{}
		virtual ~UARequest(){ /*DBG("~UARequest({:x})", (uint)this);*/ }
//		virtual ~UARequest(){};
		HCoroutine CoHandle;
	};

	Τ struct TUARequest : UARequest{
		TUARequest( T&& args, HCoroutine&& h )ι:UARequest{move(h)}, Args{move(args)}{}
		T Args;
	};
	Τ struct UARequestMulti{
		flat_map<UA_UInt32, NodeId> Requests;
		sp<UAClient> ClientPtr;
		flat_map<NodeId, T> Results;
	};

	struct UAClient final : std::enable_shared_from_this<UAClient>
	{
		UAClient( OpcServer&& opcServer )ε;
		UAClient( str address )ε;
		~UAClient();

		operator UA_Client* ()ι{ return _ptr; }
		Ω Shutdown()ι->void;
		Ω GetClient( string id, SRCE )ι->ConnectAwait{ return ConnectAwait{move(id), sl}; }
		Ω Find( str id )ι->sp<UAClient>;
		Ω Find( UA_Client* ua, SRCE )ε->sp<UAClient>;
		Ω TryFind( UA_Client* ua, SRCE )ι->sp<UAClient>;
		α SubscriptionId()Ι->SubscriptionId{ return CreatedSubscriptionResponse ? CreatedSubscriptionResponse->subscriptionId : 0;}
		α CreateSubscriptions()ι->void;
		α DataSubscriptions( CreateMonitoredItemsRequest&& r, Handle requestHandle, HCoroutine&& h )ι->void;
		α DataSubscriptionDelete( Iot::SubscriptionId subscriptionId, flat_set<MonitorId>&& monitoredItemIds )ι->void;

		//Ω GetAsyncRequest( sp<UAClient> p )ι{ lock_guard _{p->_asyncRequestMutex}; if(!p->_asyncRequest) p->_asyncRequest=ms<AsyncRequest>(move(p)); return _asyncRequest; };
		α SendBrowseRequest( Browse::Request&& request, HCoroutine&& h )ι->void;
		Ω SendReadRequest( const flat_set<NodeId>&& nodes, sp<UAClient>&& pClient, HCoroutine&& h )ι->void;
		α SendWriteRequest( flat_map<NodeId,Value>&& values, HCoroutine&& h )ι->void;
		α SetMonitoringMode( Iot::SubscriptionId subscriptionId )ι->void;
		α RequestDataTypeAttributes( const flat_set<NodeId>&& x, HCoroutine&& h )ι->void;
		Ṫ ClearRequest( UA_Client* ua, RequestId requestId )ι->up<T>;
		Ω ClearRequestH( UA_Client* ua , RequestId requestId)ι->HCoroutine{ return ClearRequest<UARequest>( ua, requestId )->CoHandle; }
		Ŧ ClearRequest( RequestId requestId )ι->up<T>;
		α ClearRequestH( RequestId requestId )ι->HCoroutine{ return ClearRequest<UARequest>( requestId )->CoHandle; }
		Ω Retry( function<void(sp<UAClient>&&, HCoroutine&&)> f, UAException e, sp<UAClient> pClient, HCoroutine h )ι->Task;
		α Process()ι->void;
		//α log()ι->void{ UA_LOG_INFO( &_config.logger, UA_LOGCATEGORY_EVENTLOOP, "Starting the EventLoop %s", "hi"); }
		//α Connect()ε->void;


		α Target()ι->str{ return _opcServer.Target; }
		α Url()ι->str{ return _opcServer.Url; }
		α IsDefault()ι->bool{ return _opcServer.IsDefault; }
		α Handle()ι->uint{ return (uint)_ptr;}
		α UAPointer()ι->UA_Client*{return _ptr;}
		sp<UA_SetMonitoringModeResponse> MonitoringModeResponse;
		sp<UA_CreateSubscriptionResponse> CreatedSubscriptionResponse;
	private:
		α Configuration()ι->UA_ClientConfig&{ return *UA_Client_getConfig(_ptr); }
		α Create()ι->UA_Client*;
		α OnSessionActivated( sp<UAClient> pClient, string id )ι->void;
		UA_ClientConfig _config{};
		OpcServer _opcServer;

		mutex _requestIdMutex;
		RequestId RequestIndex{};

		boost::concurrent_flat_map<UA_UInt32, up<UARequest>> _requests;
		boost::concurrent_flat_map<Jde::Handle, UARequestMulti<Value>> _readRequests;
		boost::concurrent_flat_map<Jde::Handle, UARequestMulti<UA_WriteResponse>> _writeRequests;
		boost::concurrent_flat_map<Jde::Handle, UARequestMulti<NodeId>> _dataAttributeRequests;

		sp<AsyncRequest> _asyncRequest; mutable mutex _asyncRequestMutex;
		UA_Client* _ptr{};//needs to be after _logContext & _config.
		friend ConnectAwait;
		friend α Read::OnResponse( UA_Client *client, void *userdata, RequestId requestId, StatusCode status, UA_DataValue *var )ι->void;
		friend α Write::OnResonse( UA_Client *ua, void *userdata, RequestId requestId, UA_WriteResponse *response )ι->void;
		friend α Attributes::OnResonse( UA_Client* ua, void* userdata, RequestId requestId, StatusCode status, UA_NodeId* dataType )ι->void;
		Logger _logger;
	public:
		Ω Unsubscribe( const sp<IDataChange>&& dataChange )ι->void;
		UAMonitoringNodes MonitoredNodes;//destroy first
	};

	Ŧ UAClient::ClearRequest( UA_Client* ua, RequestId requestId )ι->up<T>{
		up<T> request;
		try{
		 	auto p = Find( ua );
			request = p->ClearRequest<T>( requestId );
		}
		catch( const Exception& ){}
		return request;
	}

	Ŧ UAClient::ClearRequest( RequestId requestId )ι->up<T>{
		up<T> request;
	 	//lg _{ _requestMutex };
	 	if( _requests.visit( requestId, [&request](auto& x){request.reset( dynamic_cast<T*>(x.second.get()) ); if(request) x.second.release();}) ){
			_requests.erase( requestId );
			lg _{ _asyncRequestMutex };
			if( _requests.empty() )
				_asyncRequest = nullptr;
		}
		else
			CRITICAL( "Could not find handle for client={}, request={}.", Target(), requestId );
		return request;
	}
}