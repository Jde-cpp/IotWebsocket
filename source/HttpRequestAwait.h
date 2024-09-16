#pragma once
#include <jde/web/server/IHttpRequestAwait.h>
#include <jde/iot/UM.h>
#include <jde/iot/uatypes/Browse.h>

namespace Jde::Iot{
	struct NodeId; struct Value;
	using namespace Jde::Web::Server;
	struct HttpRequestAwait final: IHttpRequestAwait{
		using base = IHttpRequestAwait;
		HttpRequestAwait( HttpRequest&& req, SRCE )ι;
		α await_ready()ι->bool override;
		α Suspend()ι->void override;
		α await_resume()ε->HttpTaskResult override;
	private:
		α Login( str endpoint )ι->AuthenticateAwait::Task;
		α CoHandleRequest()ι->Jde::Task;
		α Browse()ι->Browse::ObjectsFolderAwait::Task;
		α ParseNodes()ε->tuple<flat_set<NodeId>,json>;
		α ResumeSnapshots( flat_map<NodeId, Value>&& results, json&& j )ι->void;
		α SnapshotWrite()ι->Jde::Task;
		sp<UAClient> _client;
	};
}
