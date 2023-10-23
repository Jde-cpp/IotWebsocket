#pragma once
#include "helpers.h"

namespace Jde::Iot{
	namespace Browse{ struct Response; }
	struct UAClient;
	struct NodeId;

	struct Value : UA_DataValue{
		Value( UA_StatusCode sc )ι:UA_DataValue{}{ status=sc; }
		Value( UA_DataValue& x )ι{ UA_DataValue_copy( &x, this ); }
		Value( Value&& x )ι:UA_DataValue{ x }{ Zero( x ); }
		~Value(){ UA_DataValue_clear(this); }

		α operator=( Value&& x )ι->Value&{ UA_DataValue_copy( &x, this ); return *this; }
		α ToJson()Ε->json;
	};

	namespace Read{
		struct Await final : IAwait
		{
			Await( flat_set<NodeId>&& x, sp<UAClient>&& c, SRCE )ι;
			α await_suspend( HCoroutine h )ι->void override;
			//α await_resume()ι->AwaitResult override{ return _pPromise->get_return_object().Result(); }
		private:
			flat_set<NodeId> _nodes;
			sp<UAClient> _client;
		};

		Ξ SendRequest( flat_set<NodeId>&& x, sp<UAClient>&& c )ι->Await{ return Await{ move(x), move(c) }; }
		//α Execute( sp<UAClient> ua, NodeId node, Web::Rest::Request req, bool snapShot )ι->Task;
		α OnResponse( UA_Client *client, void *userdata, UA_UInt32 requestId, UA_StatusCode status, UA_DataValue *var )ι->void;

	}
}