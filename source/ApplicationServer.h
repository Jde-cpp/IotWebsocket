#pragma once
#include <jde/web/server/IAppServer.h>
#include <jde/framework/coroutine/Await.h>
#include <jde/web/server/Sessions.h>

namespace Jde::Opc{
	struct ApplicationServer final : Web::Server::IAppServer{
		α GraphQL( string&& q, UserPK userPK, bool returnRaw, SL sl )ι->up<TAwait<jvalue>> override;
		α SessionInfoAwait( SessionPK sessionPK, SL sl )ι->up<TAwait<Web::FromServer::SessionInfo>> override;
		α IsLocal()ι->bool{ return false; }
	};
}