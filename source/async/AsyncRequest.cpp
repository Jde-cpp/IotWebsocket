#include "AsyncRequest.h"
#include "../uatypes/UAClient.h"

namespace Jde::Iot{
	static sp<LogTag> _logLevel{ Logging::TagLevel("requests") };
	flat_map<Handle,atomic_flag*> _stopProcessingLoops; shared_mutex _stopProcessingLoopMutex;
	flat_map<UA_Client*,flat_set<uint>> _processingLoopThreadIds; mutex _processingLoopThreadIdsMutex;
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
			bool isLoopThread;
			{
				lg _{ _processingLoopThreadIdsMutex };
				auto p = _processingLoopThreadIds.emplace( pClient->UAPointer(), flat_set<uint>{} ).first;
				isLoopThread = !p->second.emplace( Threading::GetThreadId() ).second;
				LOG( "ThreadId({:x}), isLoopThread={}, ", Threading::GetThreadId(), isLoopThread );
			}
			if( auto sc = isLoopThread ? 0 : UA_Client_run_iterate(*pClient, 0); sc /*&& (sc!=UA_STATUSCODE_BADINTERNALERROR || i!=0)*/ ){
				ERR( "UA_Client_run_iterate returned ({:x}){}", sc, UAException::Message(sc) );
				co_return;
			}
			{
				lg _{ _processingLoopThreadIdsMutex };
				if( auto p = _processingLoopThreadIds.find( pClient->UAPointer() ); p!=_processingLoopThreadIds.end() ){
					p->second.erase( Threading::GetThreadId() );
					LOG( "~ThreadId({:x}) deleting={}", Threading::GetThreadId(), p->second.empty() );
					if( p->second.empty() )
						_processingLoopThreadIds.erase( p );
				}
			}
			if( !stop.test() )
			{
				co_await Threading::Alarm::Wait( 500ms ); //UA_CreateSubscriptionRequest_default
				Threading::SetThreadDscrptn( "ProcessingLoop" );
			}
		}
	}
	atomic<Handle> AsyncRequest::_index{};
	AsyncRequest::AsyncRequest( sp<UAClient> pClient )ι:_pClient{move(pClient)},_handle{++_index}{}

	α AsyncRequest::Process()ι->void{
		ASSERT( _pClient );
		ProcessingLoop( _handle, move(_pClient) );
	}

	AsyncRequest::~AsyncRequest(){
		ul _{ _stopProcessingLoopMutex };
		LOG( "{}~AsyncRequest",  _handle );
		if( auto p = _stopProcessingLoops.find(_handle); p!=_stopProcessingLoops.end() )
			p->second->test_and_set();
		else
			CRITICAL( "Lost processing loop: {}", _handle );
	}
}