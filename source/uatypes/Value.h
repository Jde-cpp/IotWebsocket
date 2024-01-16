#pragma once
#include "helpers.h"

namespace Jde::Iot{
	namespace Browse{ struct Response; }
	struct UAClient;
	struct NodeId;
	struct Variant;

	struct Value : UA_DataValue{
		Value( StatusCode sc )ι:UA_DataValue{}{ status=sc; }
		Value( const UA_DataValue& x )ι{ UA_DataValue_copy( &x, this ); }
		Value( const Value& x )ι{ UA_DataValue_copy( &x, this ); }
		Value( Value&& x )ι:UA_DataValue{x}{ UA_DataValue_init(&x); }
		~Value(){ UA_DataValue_clear(this); }

		α operator=( Value&& x )ι->Value&{ UA_DataValue_copy( &x, this ); return *this; }
		α IsScaler()Ι->bool{ return UA_Variant_isScalar( &value ); }
		α ToProto( const OpcId& opcId, const NodeId& nodeId )Ι->FromServer::MessageUnion;
		α ToJson()Ι->json;
		α Set( json j )ε->void;
		Ŧ Get( uint index )Ι->const T&{ return ((T*)value.data)[index]; };
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

		Ξ SendRequest( flat_set<NodeId> x, sp<UAClient> c )ι->Await{ return Await{ move(x), move(c) }; }
		α OnResponse( UA_Client *client, void *userdata, RequestId requestId, StatusCode status, UA_DataValue *var )ι->void;
	}
}