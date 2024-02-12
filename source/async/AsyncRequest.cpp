#include "AsyncRequest.h"
#include "../uatypes/UAClient.h"

namespace Jde::Iot{
	static sp<LogTag> _logTag{ Logging::Tag("app.processingLoop") };
	α AsyncRequest::LogTag()ι->sp<Jde::LogTag>{ return _logTag; }

	// 1 per UAClient
	α AsyncRequest::ProcessingLoop()ι->Task{
		auto logPrefix = format( "[{:x}]", _pClient->Handle() );
		DBG( "{}ProcessingLoop started", logPrefix );
		while( _running.test() ){
			if( auto sc = UA_Client_run_iterate(*_pClient, 0); sc /*&& (sc!=UA_STATUSCODE_BADINTERNALERROR || i!=0)*/ ){
				ERR( "{}UA_Client_run_iterate returned ({:x}){}", logPrefix, sc, UAException::Message(sc) );
				break;
			}
			if( !_running.test() )
				break;
			co_await Threading::Alarm::Wait( 500ms ); //UA_CreateSubscriptionRequest_default
			Threading::SetThreadDscrptn( "ProcessingLoop" );
		}
		DBG( "{}ProcessingLoop stopped", logPrefix );
	}

	α AsyncRequest::Process( RequestId requestId, up<UARequest>&& userData )ι->void{
		if( _stopped.test() ) return;
		{
			lg _{_requestMutex};
			_requests.emplace( requestId, move(userData) );
		}
		if( !_running.test_and_set() )
			ProcessingLoop();
	}

	α AsyncRequest::Stop()ι->void{
		_stopped.test_and_set();
		_requests.clear();
		_running.clear();
		_pClient=nullptr;
	}
	α AsyncRequest::UAHandle()ι->Handle{ return _pClient ? _pClient->Handle() : 0; }
}