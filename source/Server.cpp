#include "Server.h"
#include "ServerSocketSession.h"
#include "ApplicationServer.h"

namespace Jde{
	optional<std::jthread> _serverThread;
	concurrent_flat_map<uint,sp<Iot::ServerSocketSession>> _sessions;

	α Iot::StartWebServer()ε->void{
		up<IException> pException;
		_serverThread = std::jthread{ [&pException]{
			try{
				Web::Server::Start( mu<RequestHandler>(), mu<ApplicationServer>() ); //blocking
			}
			catch( IException& e ){
				pException = e.Move();
			}
		} };
		while( !Web::Server::HasStarted() && !pException )
			std::this_thread::sleep_for( 100ms );
		if( pException )
		  pException->Throw();
		else
			IApplication::AddShutdownFunction( [](){Iot::StopWebServer();} );
	}

	α Iot::StopWebServer()ι->void{
		Web::Server::Stop();
		if( _serverThread && _serverThread->joinable() ){
			_serverThread->join();
			_serverThread = {};
		}
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