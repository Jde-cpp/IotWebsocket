#include "Socket.h"
#include <jde/iot/async/CreateSubscriptions.h>
#include <jde/iot/async/SetMonitoringMode.h>
#include <jde/iot/async/DataChanges.h>
//#include "uatypes/UAClient.h"
#include <jde/iot/types/MonitoringNodes.h>
#define var const auto
#define _listener TcpListener::GetInstance()

namespace Jde::Iot
{
	static sp<LogTag> _logTag = Logging::Tag( "app.socket_requests" );
	static sp<LogTag> _logTagResults = Logging::Tag( "app.socket_results" );
	up<Socket> _instance;
	α Socket::Start()ι->void{ _instance = mu<Socket>(Settings::Get<PortType>("web/port").value_or(6708)); }

	Socket::Socket( PortType port )ι:
		base{ port }
	{}

	SocketSession::SocketSession( WebSocket::WebListener& server, IO::Sockets::SessionPK id, tcp::socket&& socket )ε:
		base( server, id, move(socket) )
	{}

	SocketSession::~SocketSession(){
		TRACE( "[{}]~SocketSession()", Id );
	}

	α SocketSession::Run()ι->void{
		base::Run();
		TRACE( "[{}]SocketSession::Run()", Id );
		//Server().UpdateStatus( Server() );
	}

	α SocketSession::OnAccept( beast::error_code ec )ι->void{
		TRACE( "[{}]SocketSession::OnAccept()", Id );
		Write( FromServer::ToAck(Id) );
		base::OnAccept( ec );
	}

	α SocketSession::WriteException( IException&& e, uint32 requestId )ι->Task
	{
		TRACE( "[{}]WriteException( '{}', '{}' )", Id, requestId, e.what() );
		return Write( FromServer::ToException(requestId, e.what()) );
	}

	α SocketSession::WriteException( string m, uint32 requestId )ι->Task
	{
		TRACE( "[{}]WriteException( '{}', '{}' )", Id, requestId, m );
		return Write( FromServer::ToException(requestId, move(m)) );
	}

	α SocketSession::Server()ι->Socket&
	{
		return dynamic_cast<Socket&>( _server );
	}

	α SocketSession::Subscribe( OpcId&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task
	{
		try{
			auto spSocketSession = SharedFromThis();//keep alive
			auto pClient = ( co_await UAClient::GetClient(opcId) ).SP<UAClient>();
			TRACET( UAMonitoringNodes::LogTag(), "[{:x}]Subscribe:  {}", pClient->Handle(), NodeId::ToJson(nodes).dump() );

			( co_await Iot::CreateSubscription(pClient) ).CheckError();
			up<FromServer::SubscriptionAck> y;
			try{
				y = ( co_await DataChangesSubscribe(nodes, spSocketSession, pClient) ).UP<FromServer::SubscriptionAck>();
			}
			catch( UAException& e ){
				if( !e.IsBadSession() )
					e.Throw();
			}
			if( !y ){
				co_await AwaitSessionActivation( pClient );
				y = ( co_await DataChangesSubscribe(move(nodes), spSocketSession, move(pClient)) ).UP<FromServer::SubscriptionAck>();
			}
			y->set_request_id( requestId );
			FromServer::MessageUnion m; m.set_allocated_subscription_ack( y.release() );
			Write( move(m) );
		}
		catch( IException& e ){
			WriteException( move(e), requestId );
		}
	}
	α SocketSession::Unsubscribe( OpcId&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task{
		try{
			auto spSocketSession = SharedFromThis();//keep alive
			auto pClient = ( co_await UAClient::GetClient(opcId) ).SP<UAClient>();
			auto [successes,failures] = pClient->MonitoredNodes.Unsubscribe( move(nodes), spSocketSession );
			Write( FromServer::ToUnsubscribeResult(requestId, move(successes), move(failures)) );
		}
		catch( IException& e ){
			WriteException( move(e), requestId );
		}
	}

	α SocketSession::OnRead( FromClient::Transmission t )ι->void{
		var logPrefix = format("[{:x}", Id );
		for( auto i=0; i<t.messages_size(); ++i ){
			auto& m = *t.mutable_messages( i );
			switch( m.Value_case() )
			{
			case FromClient::MessageUnion::kSessionId:
				SessionId = m.session_id();
				TRACE("{}]Set Session id:  {:x}", logPrefix, SessionId );
				break;
			case FromClient::MessageUnion::kSubscribe:{
				auto& s = *m.mutable_subscribe();
				TRACE("{}.{}.{}]Subscribe:  {:x}", logPrefix, s.opc_id(), s.request_id(), s.nodes().size() );
				Subscribe( move(*s.mutable_opc_id()), NodeId::ToNodes(move(*s.mutable_nodes())), s.request_id() );
			}break;
			case FromClient::MessageUnion::kUnsubscribe:{
				auto& u = *m.mutable_unsubscribe(); 
				TRACE("{}.{}.{}]Unsubscribe:  {:x}", logPrefix, u.opc_id(), u.request_id(), u.nodes().size() );
				Unsubscribe( move(*u.mutable_opc_id()), NodeId::ToNodes(move(*u.mutable_nodes())), u.request_id() );
			}break;
			default:
				ERR( "Unknown message:  {}", (uint)m.Value_case() );
			}
		}
	}
	α SocketSession::Write( FromServer::MessageUnion&& m )ι->Task{
		FromServer::Transmission t;
		*t.add_messages() = move( m );
		return base::Write( move(t) );
	}
	α SocketSession::SendDataChange( const OpcId& opcId, const NodeId& node, const Value& value )ι->Task{
		return Write( value.ToProto(opcId,node) );
	}
}