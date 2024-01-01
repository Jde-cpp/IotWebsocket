#include "Socket.h"
#include "uatypes/UAClient.h"
#include "async/CreateSubscriptions.h"
#include "async/SetMonitoringMode.h"
#include "async/DataChanges.h"

#define var const auto
#define _listener TcpListener::GetInstance()

namespace Jde::Iot
{
	const LogTag& _logLevel = Logging::TagLevel( "app-webRequests" );
	Socket _instance{ Settings::Get<PortType>("web/port").value_or(6708) };

	Socket::Socket( PortType port )ι:
		base{ port }
	{}

	SocketSession::SocketSession( WebSocket::WebListener& server, IO::Sockets::SessionPK id, tcp::socket&& socket )ε:
		base( server, id, move(socket) )
	{}

	SocketSession::~SocketSession(){
		LOG( "({})~SocketSession()"sv, Id );
	}

	α SocketSession::Run()ι->void{
		base::Run();
		LOG( "({})SocketSession::Run()"sv, Id );
		//Server().UpdateStatus( Server() );
	}

	α SocketSession::OnAccept( beast::error_code ec )ι->void{
		LOG( "({})SocketSession::OnAccept()", Id );
		Write( FromServer::ToAck(Id) );
		base::OnAccept( ec );
	}

	α SocketSession::WriteException( Exception&& e, uint32 requestId )ι->Task
	{
		LOG( "({})WriteException( '{}', '{}' )"sv, Id, requestId, e.what() );
		return Write( FromServer::ToException(requestId, e.what()) );
	}

	α SocketSession::WriteException( string m, uint32 requestId )ι->Task
	{
		LOG( "({})WriteException( '{}', '{}' )"sv, Id, requestId, m );
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
			( co_await Iot::CreateSubscription(pClient) ).CheckError();

			auto y = ( co_await DataChangesSubscribe(move(nodes), spSocketSession, move(pClient)) ).UP<FromServer::SubscriptionAck>();
			y->set_request_id( requestId );
			FromServer::MessageUnion m; m.set_allocated_subscription_ack( y.release() );
			Write( move(m) );
		}
		catch( Exception& e ){
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
		catch( Exception& e ){
			WriteException( move(e), requestId );
		}
	}

	α SocketSession::OnRead( FromClient::Transmission t )ι->void{
		for( auto i=0; i<t.messages_size(); ++i ){
			auto& m = *t.mutable_messages( i );
			if( m.has_session_id() )
				SessionId = m.session_id();
			else if( auto p = m.has_subscribe() ? m.mutable_subscribe() : nullptr )
				Subscribe( move(*p->mutable_opc_id()), NodeId::ToNodes(move(*p->mutable_nodes())), p->request_id() );
			else if( auto p = m.has_unsubscribe() ? m.mutable_unsubscribe() : nullptr )
				Unsubscribe( move(*p->mutable_opc_id()), NodeId::ToNodes(move(*p->mutable_nodes())), p->request_id() );
			else
				ERR( "Unknown message:  {}"sv, (uint)m.Value_case() );
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