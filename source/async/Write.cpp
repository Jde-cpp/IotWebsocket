#include "Write.h"

namespace Jde::Iot::Write
{
	void OnResonse( UA_Client *client, void *userdata, UA_UInt32 requestId, UA_WriteResponse *response )
	{
		var handle = userdata ? (uint)userdata : requestId;
		string logPrefix = format( "[{:x}.{}.{}]", (uint)ua, handle, requestId );
		DBG( "[{}]Write::OnResponse()", logPrefix );
		auto ppClient = Try<sp<UAClient>>( [ua](){return UAClient::Find(ua);} ); if( !ppClient ) return;
		up<flat_map<NodeId, Value>> results;
		bool visited = (*ppClient)->_writeRequests.visit( handle, [requestId, sc, val, &results]( auto& pair ){
			auto& x = pair.second;
			if( auto pRequest=x.Requests.find(requestId); pRequest!=x.Requests.end() ){
				x.Results.try_emplace( pRequest->second, sc ? Value{ sc } : Value{ *val } );
				if( x.Results.size()==x.Requests.size() )
					results = mu<flat_map<NodeId, Value>>( move(x.Results) );
			}
		});
		if( !visited )
			CRITICAL( "{}Could not find handle.", logPrefix );
		else if( results ){
			(*ppClient)->_readRequests.erase( handle );
			optional<HCoroutine> h = UAClient::ClearRequest( ua, handle ); if( !h ) return CRITICAL( "Could not find handle for client={:x}, request={}.", (uint)ua, requestId );
			Resume( move(results), move(*h) );
		}
		UA_WriteResponse_clear(response);
		DBG( "[{}]Value::~OnResponse()", logPrefix );
	}
}