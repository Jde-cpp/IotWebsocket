#include "Write.h"
#define var const auto

namespace Jde::Iot::Write
{
	α OnResonse( UA_Client *ua, void *userdata, RequestId requestId, UA_WriteResponse *response )ι->void{
		var handle = userdata ? (uint)userdata : requestId;
		string logPrefix = format( "[{:x}.{}.{}]", (uint)ua, handle, requestId );
		DBG( "[{}]Write::OnResponse()", logPrefix );
		auto ppClient = Try<sp<UAClient>>( [ua](){return UAClient::Find(ua);} ); if( !ppClient ) return;
		up<flat_map<NodeId, UA_WriteResponse>> results;
		bool visited = (*ppClient)->_writeRequests.visit( handle, [requestId, response, &results]( auto& pair ){
			auto& x = pair.second;
			if( auto pRequest=x.Requests.find(requestId); pRequest!=x.Requests.end() ){
				x.Results.try_emplace( pRequest->second, *response ); memset( response, 0, sizeof(UA_WriteResponse) );
				if( x.Results.size()==x.Requests.size() )
					results = mu<flat_map<NodeId, UA_WriteResponse>>( move(x.Results) );
			}
		});
		if( !visited )
			CRITICAL( "{}Could not find handle.", logPrefix );
		else if( results ){
			(*ppClient)->_readRequests.erase( handle );
			HCoroutine h = (*ppClient)->ClearRequestH( handle ); if( !h ) return CRITICAL( "Could not find handle for client={:x}, request={}.", (uint)ua, requestId );
			Resume( move(results), move(h) );
		}
		DBG( "[{}]Value::~OnResponse()", logPrefix );
	}
}