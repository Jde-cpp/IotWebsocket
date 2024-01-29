#include "ConnectAwait.h"
#include "../uatypes/UAClient.h"

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
		if( auto pClient = UAClient::Find(_id); pClient )
			Jde::Resume( move(pClient), move(h) );
		else{
			_requestMutex.lock();
			auto p = _requests.find( _id );
			if( p==_requests.end() ){
				_requests[_id] = {h};
				_requestMutex.unlock();
				Create( _id );
			}
			else{
				p->second.push_back( move(h) );
				_requestMutex.unlock();
			}
	
		}
	}
	α ConnectAwait::Create( string id )ι->Task{
		try{
			auto pServer = ( co_await OpcServer::Select(id) ).UP<OpcServer>(); THROW_IF( !pServer, "Could not find opc server:  '{}'", id );
			auto pClient = ms<UAClient>( move(*pServer) );
			pClient->Create();
			pClient->OnSessionActivated( pClient, id );
			var sc = UA_Client_connectAsync( *pClient, pClient->Url().c_str() ); THROW_IFX( sc, UAException(sc) );
			pClient->Process();
		}
		catch( const Exception& e ){
			lg _{ _requestMutex };
			var ua = dynamic_cast<const UAException*>( &e );
			for( auto& r : _requests[id] ){
				Jde::Resume( ua ? UAException{*ua} : Exception{e.what(), e.Code, e.Level(), e.Stack().front()}, move(r) );
			}
			_requests.erase( id );
		}
	}
	α ConnectAwait::Resume( sp<UAClient> pClient, string&& target, function<void(HCoroutine&&)> resume )ι->void{
		ASSERT( pClient );
		pClient->_asyncRequest = nullptr;
		lg _{ _requestMutex };
		for( auto h : _requests[target] )
			resume( move(h) );
		_requests.erase( target );
	}

	α ConnectAwait::Resume( sp<UAClient>&& pClient, string&& target )ι->void{
		Resume( pClient, move(target), [p=pClient](HCoroutine&& h){ Jde::Resume((sp<UAClient>)move(p), move(h)); } );
	}
	α ConnectAwait::Resume( sp<UAClient>&& pClient, string&& target, const UAException&& e )ι->void{
		Resume( move(pClient), move(target), [sc=e.Code](HCoroutine&& h){ Jde::Resume(UAException{(StatusCode)sc}, move(h)); } );
	}
}