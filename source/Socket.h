#pragma once
#include "../../Public/src/web/WebSocket.h"
#include "../../Framework/source/collections/UnorderedSet.h"
//#include "types/MonitoringNodes.h"
#include <jde/iot/uatypes/Node.h>
#include <jde/iot/uatypes/UAClient.h>

namespace Jde::Iot{
	namespace beast = boost::beast;
	using tcp = boost::asio::ip::tcp;
	struct Socket; struct Value;

	struct SocketSession final : WebSocket::TSession<FromServer::Transmission,FromClient::Transmission>, IDataChange{
		typedef WebSocket::TSession<FromServer::Transmission,FromClient::Transmission> base;
		SocketSession( WebSocket::WebListener& server, IO::Sockets::SessionPK id, tcp::socket&& socket )ε;
		~SocketSession();
		α OnDisconnect( CodeException&& )ι->void override{ UAClient::Unsubscribe( dynamic_pointer_cast<IDataChange>(shared_from_this())); }
		α OnRead( FromClient::Transmission transmission )ι->void override;
		α OnAccept( beast::error_code ec )ι->void override;
		α WriteCustom( uint32 id, string&& m )ι->Task;
		α Run()ι->void override;
		α SendDataChange( const OpcNK& opcId, const NodeId& node, const Value& value )ι->Task override;
		α to_string()Ι->string override{ return format( "{:x}", Id ); }
	private:
		α WriteException( IException&& e, uint32 requestId )ι->Task;
		α WriteException( string msg, uint32 requestId=0 )ι->Task;
		α Write( IException&& e, uint32 requestId )ι->Task{ return WriteException( e.What(), requestId ); }
		α Write( FromServer::MessageUnion&& m )ι->Task;
		α Server()ι->Socket&;
		α SharedFromThis()ι->sp<SocketSession>{ return dynamic_pointer_cast<SocketSession>(shared_from_this()); }
		α Subscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task;
		α Unsubscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task;
		uint32 SessionId{0};//From rest.
	};

	struct Socket final : WebSocket::TListener<FromServer::Transmission,SocketSession>
	{
		using base=WebSocket::TListener<FromServer::Transmission,SocketSession>;
		Socket( PortType port )ι;
		Ω Start()ι->void;
	};
}