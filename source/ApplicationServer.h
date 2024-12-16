#pragma once
#include <jde/web/server/IApplicationServer.h>
namespace Jde::Opc{
	struct ApplicationServer final : Web::Server::IApplicationServer{
		α GraphQL( string&& q, UserPK userPK, SL sl )ι->up<TAwait<json>> override;
		α SessionInfoAwait( SessionPK sessionPK, SL sl )ι->up<TAwait<Web::Server::SessionInfo>> override;
	};
}