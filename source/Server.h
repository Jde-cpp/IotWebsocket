#pragma once
#include <jde/web/server/Flex.h>
#include "HttpRequestAwait.h"

namespace Jde::Iot{
	α StartWebServer()ε->void;
	α StopWebServer()ι->void;
namespace Server{
	α RemoveSession( uint socketSessionId )ι->void;
}
	struct RequestHandler final : IRequestHandler{
		α HandleRequest( HttpRequest&& req, SRCE )ι->up<IHttpRequestAwait> override{ return mu<HttpRequestAwait>( move(req), sl ); }
		α RunWebsocketSession( sp<RestStream>&& stream, beast::flat_buffer&& buffer, TRequestType req, tcp::endpoint userEndpoint, uint32 connectionIndex )ι->void override;
	};
}