#pragma once
#include "Node.h"
#include "helpers.h"
#include "Value.h"

namespace Jde::Iot{
	struct UAClient;
//	struct Value;
namespace Browse{

	α ObjectsFolder( sp<UAClient> ua, NodeId node, Web::Rest::Request req, bool snapShot )ι->Task;
	α OnResponse( UA_Client *ua, void* userdata, UA_UInt32 requestId, UA_BrowseResponse* response )ι->void;

	struct FoldersAwait final : IAwait
	{
		FoldersAwait( NodeId&& node, sp<UAClient>& c, SRCE )ι:IAwait{sl}, _node{move(node)},_client{c}{}
		α await_suspend( HCoroutine h )ι->void override;
		α await_resume()ι->AwaitResult override{ return _pPromise->get_return_object().Result(); }
	private:
		NodeId _node;
		sp<UAClient> _client;
	};

	struct Request :UA_BrowseRequest{
		Request( NodeId&& node )ι;
		Request( Request&& x )ι:UA_BrowseRequest{ x }{ Zero( x );}
		Request( const Request& x )ι{ UA_BrowseRequest_copy( &x, this ); }
		~Request(){ UA_BrowseRequest_clear(this); }
	};

	struct Response : UA_BrowseResponse{
		Response( UA_BrowseResponse&& x )ι:UA_BrowseResponse{ x }{ Zero( x ); }
		Response( Response&& x )ι:UA_BrowseResponse{ *this }{ Zero( *this ); }
		~Response(){ UA_BrowseResponse_clear(this); }

		α ToJson( sp<flat_map<tuple<uint,uint>, Value>>&& pValues )ε->json;
	};
}}