#pragma once
#include "../../Framework/source/net/WebSocket.h"

namespace Jde::Iot
{
	namespace beast = boost::beast;
	using tcp = boost::asio::ip::tcp;
	struct WebServer;
	struct MySession final : WebSocket::TSession<FromServer::Transmission,FromClient::Transmission>
	{
		typedef WebSocket::TSession<FromServer::Transmission,FromClient::Transmission> base;
		MySession( WebSocket::WebListener& server, IO::Sockets::SessionPK id, tcp::socket&& socket )ε;
		~MySession();
		α OnConnect()ι->void;
		α OnRead( FromClient::Transmission transmission )ι->Task override;
		α OnAccept( beast::error_code ec )ι->void override;
		α SendStatuses()ι->void;
		//Ω SendLogs( sp<MySession> self, ApplicationPK applicationId, ApplicationInstancePK instanceId, ELogLevel level, time_t start, uint limit )ι->void;
		//α PushMessage( LogPK id, ApplicationInstancePK applicationId, ApplicationInstancePK instanceId, TimePoint time, ELogLevel level, uint32 messageId, uint32 fileId, uint32 functionId, uint16 lineNumber, uint32 userId, uint threadId, const vector<string>& variables )ι->void;
		//α WriteCustom( uint32 clientId, const string& message )ι->void;
		α WriteCustom( uint32 id, string&& m )ι->Task;
		//α WriteComplete( uint32 clientId )ι->void;
		α Run()ι->void override;
	private:
		α SendStrings( const FromClient::RequestStrings& request )ι->void;
		α WriteError( string msg, uint32 requestId=0 )ι->Task;
		α Write( IException&& e, uint32 requestId )ι->Task{ return WriteError( e.What(), requestId ); }
		//α Write( const FromServer::Transmission& tranmission )ε->void;
		α Write( FromServer::MessageUnion&& m )ι->Task;
		α Server()ι->WebServer&;
	};
}