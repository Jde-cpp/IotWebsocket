#include "AsyncRequest.h"
#include "../uatypes/UAClient.h"
#include "../uatypes/UAException.h"

namespace Jde::Iot{

	flat_map<Handle,atomic_flag*> _stopProcessingLoops; shared_mutex _stopProcessingLoopMutex;
	α ProcessingLoop( Handle handle, sp<UAClient> pClient )ι->Task
	{
		atomic_flag stop;
		{
			ul _{ _stopProcessingLoopMutex };
			ASSERT_DESC( _stopProcessingLoops.find(handle)==_stopProcessingLoops.end(), format("Duplicated processing loop: {}", handle) );
			_stopProcessingLoops[handle]=&stop;
		}
		while( !stop.test() )
		{
			auto result = UA_Client_run_iterate( *pClient, 0 );
			if( result!=UA_STATUSCODE_GOOD )
				ERR( "UA_Client_run_iterate returned ({}){}", result, UAException::Message(result) );
			if( !stop.test() )
			{
				co_await Threading::Alarm::Wait( 100ms );
				Threading::SetThreadDscrptn( "ProcessingLoop" );
			}
		}
	}
	atomic<Handle> AsyncRequest::_index{};
	AsyncRequest::AsyncRequest( sp<UAClient> pClient )ι:_pClient{move(pClient)},_handle{++_index}
	{}

	α AsyncRequest::Process()ι->void{
		ASSERT(_pClient);
		ProcessingLoop(_handle,move(_pClient));
	}

	AsyncRequest::~AsyncRequest(){
		ul _{ _stopProcessingLoopMutex };
		DBG( "{}~AsyncRequest",  _handle );
		if( auto p = _stopProcessingLoops.find(_handle); p!=_stopProcessingLoops.end() )
			p->second->test_and_set();
		else
			CRITICAL( "Lost processing loop: {}", _handle );
	}
}