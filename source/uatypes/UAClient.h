#pragma once
#include "Logger.h"
#include "../types/OpcServer.h"
#include "../async/AsyncRequest.h"
#include "../async/ConnectAwait.h"

namespace Jde::Iot
{
	// //todo look at CacheAwait
	// struct ClientAwaitable final : IAwaitCache
	// {
	// 	ClientAwaitable( string id ):_id{move(id)} {}
	// 	α await_ready()ι->bool override;
	// 	α await_suspend( HCoroutine h )ι->void override;
	// private:
	// 	string _id;
	// };
	struct UAClient final : std::enable_shared_from_this<UAClient>
	{
		UAClient( OpcServer&& opcServer )ε;
		UAClient( str address )ε;
		~UAClient(){ DBG("~UAClient( ({:X}){} ) )", Handle(), Target()); UA_Client_delete(_ptr); }

		operator UA_Client* ()ι{ return _ptr; }

		Ω GetClient( string id, SRCE )ι->ConnectAwait{ return ConnectAwait{move(id), sl}; }
		Ω Find( str id )ι->sp<UAClient>;
		Ω Find( UA_Client* p )ε->sp<UAClient>;

		//Ω GetAsyncRequest( sp<UAClient> p )ι{ lock_guard _{p->_asyncRequestMutex}; if(!p->_asyncRequest) p->_asyncRequest=ms<AsyncRequest>(move(p)); return _asyncRequest; };

    α SendBrowseRequest( UA_BrowseRequest bReq, HCoroutine h )ι->Task;
		Ω ClearRequest( UA_Client* ua, UA_UInt32 requestId )ι->optional<HCoroutine>;
		α Process()ι->void;
		//α log()ι->void{ UA_LOG_INFO( &_config.logger, UA_LOGCATEGORY_EVENTLOOP, "Starting the EventLoop %s", "hi"); }
		//α Connect()ε->void;


		α Target()ι->str{ return _opcServer.Target; }
		α Url()ι->str{ return _opcServer.Url; }
		α IsDefault()ι->bool{ return _opcServer.IsDefault; }
		α Handle()ι->uint{ return (uint)_ptr;}
		α UAPointer()ι->UA_Client*{return _ptr;}
		UA_UInt32 RequestId{};
	private:
		α Configuration()ι->UA_ClientConfig&{ return *UA_Client_getConfig(_ptr); }
		α Create()ι->UA_Client*;
		α OnSessionActivated( sp<UAClient> pClient, string id )ι->void;
		UA_ClientConfig _config{};
		void* _logContext{};
		OpcServer _opcServer;
		flat_map<UA_UInt32, HCoroutine> _requests; mutex _requestMutex;
		sp<AsyncRequest> _asyncRequest; mutable mutex _asyncRequestMutex;
		UA_Client* _ptr{};//needs to be after _logContext & _config.
		friend ConnectAwait;
	};
}