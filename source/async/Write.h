#pragma once
#include "../uatypes/helpers.h"
#include "../uatypes/UAClient.h"
#include "../uatypes/Value.h"

namespace Jde::Iot::Write{
	struct Await final : IAwait
	{
		Await( flat_map<NodeId,Value>&& x, sp<UAClient>&& c, SRCE )ι:IAwait{sl}, _values{move(x)},_client{move(c)}{}
		α await_suspend( HCoroutine h )ι->void override{ IAwait::await_suspend( h ); _client->SendWriteRequest( move(_values), move(h) ); }
	private:
		flat_map<NodeId,Value> _values;
		sp<UAClient> _client;
	};
	Ξ SendRequest( flat_map<NodeId,Value>&& x, sp<UAClient> c, SRCE )ι->Await{ return Await{ move(x), move(c), sl }; }
	α OnResonse( UA_Client *client, void *userdata, RequestId requestId, UA_WriteResponse *response )ι->void;
}