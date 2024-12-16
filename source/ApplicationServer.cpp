#include "ApplicationServer.h"
#include <jde/app/client/await/SocketAwait.h>

namespace Jde::Opc{
	α ApplicationServer::GraphQL( string&& q, UserPK userPK, SL sl )ι->up<TAwait<json>>{
		return mu<App::Client::GraphQLAwait>( move(q), userPK, sl );
	}
	α ApplicationServer::SessionInfoAwait( SessionPK sessionPK, SL sl )ι->up<TAwait<Web::Server::SessionInfo>>{
		return mu<App::Client::SessionInfoAwait>( sessionPK, sl );
	}
}