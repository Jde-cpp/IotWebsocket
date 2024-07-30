#pragma once
#include <jde/web/server/IHttpRequestAwait.h>

namespace Jde::Iot{
	struct NodeId; struct Value;
	using namespace Jde::Web::Server;
	struct HttpRequestAwait final: IHttpRequestAwait{
		using base = IHttpRequestAwait;
		HttpRequestAwait( HttpRequest&& req, SRCE )ι;
		α await_ready()ι->bool override;
		α await_suspend( base::Handle h )ε->void;
		α await_resume()ε->HttpTaskResult override;
	private:
		α Login()ι->Jde::Task;
		α CoHandleRequest()ι->Jde::Task;
		α ParseNodes()ε->tuple<flat_set<NodeId>,json>;
		α SetSnapshotResult( flat_map<NodeId, Value>&& results, json&& j )ι->void;
	};
}
