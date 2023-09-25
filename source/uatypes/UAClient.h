#pragma once
#include "Logger.h"
#include "../types/OpcServer.h"

namespace Jde::Iot
{
	//todo look at CacheAwait
	struct ClientAwaitable final : IAwaitCache
	{
		ClientAwaitable( string id ):_id{move(id)} {}
		α await_ready()ι->bool override;
		α await_suspend( HCoroutine h )ι->void override;
	private:
		string _id;
	};
	struct UAClient;
	struct AsyncRequest final
	{
		AsyncRequest( sp<UAClient>&& pClient )ι;
		~AsyncRequest();
		private:
		Handle _handle;
		static atomic<Handle> _index;
	};
	struct UAClient final /*: std::enable_shared_from_this<UAClient>*/
	{
		UAClient( sp<OpcServer> pOpcServer )ε;
		UAClient( string address )ε;
		~UAClient(){ UA_Client_delete(_ptr); }

		operator UA_Client* ()ι{ return _ptr; }

		Ω GetClient( string id )ι->ClientAwaitable{ return {move(id)}; }
		Ω GetAsyncRequest( sp<UAClient> x )ι{ lock_guard _{_asyncRequestMutex}; if(!_asyncRequest) _asyncRequest=ms<AsyncRequest>(move(x)); return _asyncRequest; };

		//α log()ι->void{ UA_LOG_INFO( &_config.logger, UA_LOGCATEGORY_EVENTLOOP, "Starting the EventLoop %s", "hi"); }
		α Connect()ε->void;
		α Configuration()ι->UA_ClientConfig&{ return *UA_Client_getConfig(_ptr); }
		UA_UInt32 RequestId; mutex RequestIdMutex;
	private:
		α Create()ι->UA_Client*; //ua_config_default.c

		UA_ClientConfig _config{};
		void* _logContext{};
		sp<OpcServer> _pOpcServer;
		static sp<AsyncRequest> _asyncRequest; static mutex _asyncRequestMutex;
		UA_Client* _ptr;//needs to be after _logContext & _config.
	};
}