#include "Server.h"
#include "ServerSocketSession.h"
#include "ApplicationServer.h"

namespace Jde{
	optional<std::jthread> _serverThread;
	concurrent_flat_map<uint,sp<Iot::ServerSocketSession>> _sessions;

	α Iot::StartWebServer()ε->void{
		Web::Server::Start( mu<RequestHandler>(), mu<ApplicationServer>() );
		Process::AddShutdownFunction( [](bool /*terminate*/ ){Iot::StopWebServer();} );//TODO move to Web::Server
	}

	α Iot::StopWebServer()ι->void{
		Web::Server::Stop();
	}
namespace Iot{
	α Server::RemoveSession( uint socketSessionId )ι->void{
		_sessions.erase( socketSessionId );
	}

	α RequestHandler::RunWebsocketSession( sp<RestStream>&& stream, beast::flat_buffer&& buffer, TRequestType req, tcp::endpoint userEndpoint, uint32 connectionIndex )ι->void{
		auto pSession = ms<ServerSocketSession>( move(stream), move(buffer), move(req), move(userEndpoint), connectionIndex );
		_sessions.emplace( pSession->Id(), pSession );
		pSession->Run();
	}
}}