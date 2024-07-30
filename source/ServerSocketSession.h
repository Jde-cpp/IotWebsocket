#pragma once
#include <jde/web/server/Sessions.h>
#include <jde/web/client/usings.h>
#include <jde/iot/types/MonitoringNodes.h>
#include <jde/web/server/IWebsocketSession.h>

namespace Jde::Iot{
	using namespace Jde::Web::Server;
	using namespace Jde::Web::Client;
	struct NodeId;
	struct ServerSocketSession : TWebsocketSession<FromServer::Transmission,FromClient::Transmission>, IDataChange{
		using base = TWebsocketSession<FromServer::Transmission,FromClient::Transmission>;
		ServerSocketSession( sp<RestStream> stream, beast::flat_buffer&& buffer, TRequestType&& request, tcp::endpoint&& userEndpoint, uint32 connectionIndex )ι;
		α OnRead( FromClient::Transmission&& transmission )ι->void override;
		α SendDataChange( const Jde::Iot::OpcNK& opcNK, const Jde::Iot::NodeId& node, const Jde::Iot::Value& value )ι->void override;
		α to_string()Ι->string override{ return 𐢜( "{:x}", Id() ); }
	private:
		α OnClose()ι->void;
		//α OnConnect( SessionPK sessionId, RequestId requestId )ι->Web::UpsertAwait::Task;
		α ProcessTransmission( FromClient::Transmission&& transmission )ι->void;
		α SetSessionId( SessionPK sessionId, RequestId requestId )->Sessions::UpsertAwait::Task;
		α SharedFromThis()ι->sp<ServerSocketSession>{ return std::dynamic_pointer_cast<ServerSocketSession>(shared_from_this()); }
		α Subscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task;
		α Unsubscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task;
		α WriteException( IException&& e, RequestId requestId )ι->void;
		α WriteException( IException&& e )ι->void override{ WriteException( move(e), 0 ); }

		α GraphQL( string&& query, uint requestId )ι->Task;
		α SendAck( uint id )ι->void override;
	};
}

