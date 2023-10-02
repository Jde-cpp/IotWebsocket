#include "ConnectAwait.h"
#include "../uatypes/UAClient.h"
#include "../uatypes/UAException.h"

#define var const auto

namespace Jde::Iot
{
	flat_map<string,vector<HCoroutine>> _requests; mutex _requestMutex;

	α ConnectAwait::await_ready()ι->bool{
		_result.Set( UAClient::Find(_id) );
		return _result.HasShared();
	}

	α ConnectAwait::await_suspend( HCoroutine h )ι->void{
		IAwait::await_suspend( h );
		lg _{ _requestMutex };
		if( auto pClient = UAClient::Find(_id); pClient )
			ResumeSP( move(h), pClient );
		else{
			auto p = _requests.find( _id );
			if( p==_requests.end() ){
				_requests[_id] = {h};
				_requestMutex.unlock();
				Create( _id );
			}
			else
				p->second.push_back( move(h) );
		}
	}
	α ConnectAwait::Create( string id )ι->Task{
		try{
			auto pServer = ( co_await OpcServer::Select(id) ).UP<OpcServer>(); THROW_IF( !pServer, "Could not find opc server:  '{}'", id );
			auto pClient = ms<UAClient>( move(*pServer) );
			pClient->Create();
			DBG( "created client ({:x})[{}]{}", pClient->Handle(), pClient->Target(), pClient->Url() );
			pClient->OnSessionActivated( pClient, id );
			var sc = UA_Client_connectAsync( *pClient, pClient->Url().c_str() ); THROW_IFX( sc, UAException(sc) );
			pClient->Process();
		}
		catch( const Exception& e ){
			lg _{ _requestMutex };
			var ua = dynamic_cast<const UAException*>( &e );
			for( auto& r : _requests[id] ){
				ResumeEx( move(r), ua ? UAException{*ua} : Exception{e.what(), e.Code, e.Level(), e.Stack().front()} );
			}
			_requests.erase( id );
		}
	}
	α ConnectAwait::Resume( sp<UAClient> pClient, string id )ι->void{
		lg _{ _requestMutex };
		for( auto r : _requests[id] )
			ResumeSP( move(r), pClient );
		_requests.erase( id );
	}
}