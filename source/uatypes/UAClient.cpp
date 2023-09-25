#include "UAClient.h"
#include "UAException.h"

#define var const auto
namespace Jde::Iot
{
	sp<AsyncRequest> UAClient::_asyncRequest{};
	mutex UAClient::_asyncRequestMutex{};
	UAClient::UAClient( string address )ε:
		UAClient{ ms<OpcServer>(move(address)) }
	{}

	UAClient::UAClient(sp<OpcServer> pOpcServer)ε:
		_pOpcServer{ pOpcServer },
		_ptr{ Create() }
	{
		UA_ClientConfig_setDefault( &Configuration() );
		//_ptr->log();
		Connect();
	}

	α UAClient::Create()ι->UA_Client* //ua_config_default.c
	{
		//      memset( &_config, 0, sizeof(UA_ClientConfig) );
		//UA_Logger logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
		_config.logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
		//UA_Log_Stdout_withLevel(UA_LOGLEVEL_INFO);
		_config.eventLoop = UA_EventLoop_new_POSIX(&_config.logger);
		UA_ConnectionManager *tcpCM = UA_ConnectionManager_new_POSIX_TCP( "tcp connection manager"_uv );
		_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)tcpCM);
		_config.timeout = 10;

		UA_ConnectionManager *udpCM = UA_ConnectionManager_new_POSIX_UDP( "udp connection manager"_uv );
		_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)udpCM);
		UA_Client *c = UA_Client_newWithConfig(&_config);
		UA_Client_getConfig(c)->eventLoop->logger = &_config.logger;

		return c;
	}

	α UAClient::Connect()ε->void
	{
		if( UA_StatusCode retval = UA_Client_connect( _ptr, _pOpcServer->Address.c_str() ); retval != UA_STATUSCODE_GOOD )
			throw (int)retval;
	}
	flat_map<Handle,atomic_flag*> _stopProcessingLoops; shared_mutex _stopProcessingLoopMutex;
	α ProcessingLoop( Handle handle, sp<UAClient>&& pClient )ι->Task
	{
		atomic_flag stop;
		{
			ul _{ _stopProcessingLoopMutex };
			ASSERT_DESC( _stopProcessingLoops.find(handle)==_stopProcessingLoops.end(), format("Duplicated processing loop: {}", handle) );
			_stopProcessingLoops[handle]=&stop;
		}
		for(;;)
		{
			auto result = UA_Client_run_iterate( *pClient, 0 );
			if( result!=UA_STATUSCODE_GOOD )
				ERR( "UA_Client_run_iterate returned ({}){}", result, UAException::Message(result) );
			if( !stop.test() )
				co_await Threading::Alarm::Wait( 100ms );
		}
	}
	atomic<Handle> AsyncRequest::_index{};
	AsyncRequest::AsyncRequest( sp<UAClient>&& pClient )ι:_handle{++_index}{ ProcessingLoop(_handle,move(pClient)); }
	AsyncRequest::~AsyncRequest()
	{
		ul _{ _stopProcessingLoopMutex };
		if( auto p = _stopProcessingLoops.find(_handle); p!=_stopProcessingLoops.end() )
			p->second->test_and_set();
		else
			CRITICAL( "Lost processing loop: {}", _handle );
	}

	flat_map<string,sp<UAClient>> _clients; shared_mutex _clientsMutex;
	α ClientAwaitable::await_ready()ι->bool
	{
		sl _{ _clientsMutex };
		if( auto p = _clients.find(_id); p!=_clients.end() )
			_result.Set( p->second );
		return _result.HasShared();
	}

	α GetClient( string id, HCoroutine h )ι->Task
	{
		try
		{
			h.promise().get_return_object().SetResult( (co_await OpcServer::Select(id)).UP<OpcServer>() );
		}
		catch( Exception& e )
		{
			h.promise().get_return_object().SetResult( move(e) );
		}
		h.resume();
	}

	α ClientAwaitable::await_suspend( HCoroutine h )ι->void
	{
		IAwaitCache::await_suspend( h );
		GetClient( _id, h );
	}

}