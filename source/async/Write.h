#pragma once
#include "helpers.h"
#include "../uatypes/Value.h"

namespace Jde::Iot::Write{
	struct Await final : IAwait
	{
		Await( flat_map<NodeId,value>&& x, sp<UAClient>&& c, SRCE )ι:IAwait{sl}, _values{move(x)},_client{move(c)}{}
		α await_suspend( HCoroutine h )ι->void override{ IAwait::await_suspend( h ); _client->SendWriteRequest( move(_nodes), move(_values), move(h) );
 }
	private:
		flat_map<NodeId,Value> _values;
		sp<UAClient> _client;
	};
	α OnResonse( UA_Client *client, void *userdata, UA_UInt32 requestId, UA_WriteResponse *response )ι->void;
static
void
attrWritten(UA_Client *client, void *userdata, UA_UInt32 requestId,
            UA_WriteResponse *response) {
    /*assuming no data to be retrieved by writing attributes*/
    printf("%-50s%u\n", "Wrote value attribute for request ", requestId);
    UA_WriteResponse_clear(response);
}

}