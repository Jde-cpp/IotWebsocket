#include "ApplicationServer.h"
#include <jde/app/client/await/SocketAwait.h>

namespace Jde::Iot{
	α ApplicationServer::GraphQL( string&& q, UserPK userPK, SL sl )ι->up<TAwait<json>>{
		return mu<App::Client::GraphQLAwait>( move(q), userPK, sl );
	}
	α ApplicationServer::SessionInfoAwait( SessionPK sessionPK, SL sl )ι->up<TAwait<App::Proto::FromServer::SessionInfo>>{
		return mu<App::Client::SessionInfoAwait>( sessionPK, sl );
	}
}