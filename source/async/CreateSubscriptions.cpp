#include "CreateSubscriptions.h"
#define var const auto

namespace Jde::Iot{
	boost::concurrent_flat_map<sp<UAClient>,vector<HCoroutine>> _subscriptionRequests;
	α CreateSubscriptionCallback(UA_Client* ua, void* /*userdata*/, RequestId requestId, void *response)ι->void{
		auto pResponse = static_cast<UA_CreateSubscriptionResponse*>( response );
		auto ppClient = Try<sp<UAClient>>( [ua](){return UAClient::Find(ua);} ); if( !ppClient ) return;
		(*ppClient)->ClearRequest<UARequest>( requestId );
		if( var sc = pResponse->responseHeader.serviceResult; sc )
			CreateSubscriptionAwait::Resume( sc, move(*ppClient) );
		else{
			(*ppClient)->CreatedSubscriptionResponse = ms<UA_CreateSubscriptionResponse>(move(*pResponse));
			CreateSubscriptionAwait::Resume( move(*ppClient) );
		}
	}

	α StatusChangeNotificationCallback(UA_Client* ua, UA_UInt32 subId, void *subContext, UA_StatusChangeNotification *notification)ι->void{
		BREAK;
		DBG( "StatusChangeNotificationCallback={:x}.{}", (uint)ua, subId );
	}

	α DeleteSubscriptionCallback(UA_Client* ua, UA_UInt32 subId, void *subContext)ι->void{
		DBG( "DeleteSubscriptionCallback={:x}.{}", (uint)ua, subId );
	}

	α CreateSubscriptionAwait::await_ready()ι->bool{ return _client->CreatedSubscriptionResponse!=nullptr; }
	α CreateSubscriptionAwait::await_suspend( HCoroutine h )ι->void{
		IAwait::await_suspend( h );
		if( _subscriptionRequests.try_emplace_or_visit(_client, vector<HCoroutine>{h}, [ h2=move(h)](auto x){x.second.push_back(move(h2));}) )
			_client->CreateSubscriptions();
	}

	α CreateSubscriptionAwait::await_resume()ι->AwaitResult{
		ASSERT( _client->CreatedSubscriptionResponse || (_pPromise && _pPromise->HasError()) );
		return _pPromise
			? _pPromise->MoveResult()
			: AwaitResult{ _client->CreatedSubscriptionResponse };
	}
	α CreateSubscriptionAwait::Resume( sp<UAClient> pClient, function<void(HCoroutine&&)> resume )ι->void{
		ASSERT( pClient );
		if( !_subscriptionRequests.cvisit(pClient, [resume](var& x){for( auto h : x.second ) resume( move(h) );}) )
			CRITICAL( "Could not find client ({:x}) for CreateSubscriptionAwait", (uint)pClient->UAPointer() );

		_subscriptionRequests.erase( pClient );
	}

	α CreateSubscriptionAwait::Resume( sp<UAClient> pClient )ι->void{
		Resume( pClient, [pClient](HCoroutine&& h)
		{
			h.promise().SetResult( sp<UA_CreateSubscriptionResponse>{pClient->CreatedSubscriptionResponse} );
			Coroutine::CoroutinePool::Resume( move(h) ); //Cannot run EventLoop from the run method itself
		});
	}
	α CreateSubscriptionAwait::Resume( StatusCode sc, sp<UAClient>&& pClient )ι->void{
		Resume( move(pClient), [sc](HCoroutine&& h){Jde::Resume(UAException{sc}, move(h));} );
	}
}