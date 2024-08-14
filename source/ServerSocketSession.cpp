#include "ServerSocketSession.h"
#include <jde/iot/UM.h>
#include <jde/iot/async/CreateSubscriptions.h>
#include <jde/iot/async/DataChanges.h>
#include "Server.h"
#include "types/proto/Iot.FromServer.h"

#define var const auto

namespace Jde::Iot{
	ServerSocketSession::ServerSocketSession( sp<RestStream> stream, beast::flat_buffer&& buffer, TRequestType&& request, tcp::endpoint&& userEndpoint, uint32 connectionIndex )ι:
		base{ move(stream), move(buffer), move(request), move(userEndpoint), connectionIndex }
	{}

	α ServerSocketSession::OnClose()ι->void{
		LogRead( "OnClose", 0 );
		Server::RemoveSession( Id() );
		base::OnClose();
	}

	α ServerSocketSession::SendAck( uint id )ι->void{
		LogWrite( 𐢜("Ack id: {:x}", id), 0 );
		Write( FromServer::AckTrans(id) );
	}

	α ServerSocketSession::SendDataChange( const Jde::Iot::OpcNK& opcNK, const Jde::Iot::NodeId& node, const Jde::Iot::Value& value )ι->void{
		return Write( MessageTrans(value.ToProto(opcNK,node), 0) );
	}

	α ServerSocketSession::SetSessionId( SessionPK sessionId, RequestId requestId )->Sessions::UpsertAwait::Task{
		LogRead( 𐢜("sessionId: '{:x}'", sessionId), requestId );
		try{
			co_await Sessions::UpsertAwait( 𐢜("{:x}", sessionId), _userEndpoint.address().to_string(), true );
			base::SetSessionId( sessionId );
			Write( FromServer::CompleteTrans(requestId) );
		}
		catch( IException& e ){
			WriteException( move(e), requestId );
		}
	}

	α ServerSocketSession::Subscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task{
		try{
			auto self = SharedFromThis(); //keep alive
			auto [loginName,password] = Credentials( base::SessionId(), opcId );
			LogRead( 𐢜("Subscribe: opcId: '{}', user: '{}', nodeCount: {}", opcId, loginName, nodes.size()), requestId );

			auto client = ( co_await UAClient::GetClient(move(opcId), loginName, password) ).SP<UAClient>();
			( co_await Iot::CreateSubscription(client) ).CheckError();
			up<FromServer::SubscriptionAck> ack;
			try{
				ack = ( co_await DataChangesSubscribe(nodes, self, client) ).UP<FromServer::SubscriptionAck>();
			}
			catch( UAException& e ){
				if( !e.IsBadSession() )
					e.Throw();
			}
			if( !ack ){
				co_await AwaitSessionActivation( client );
				ack = ( co_await DataChangesSubscribe(move(nodes), self, move(client)) ).UP<FromServer::SubscriptionAck>();
			}
			Write( FromServer::SubscribeAckTrans(move(ack), requestId) );
		}
		catch( IException& e ){
			WriteException( move(e), requestId );
		}
	}

	α ServerSocketSession::Unsubscribe( OpcNK&& opcId, flat_set<NodeId> nodes, uint32 requestId )ι->Task{
		try{
			auto self = SharedFromThis();//keep alive
			auto [loginName,password] = Iot::Credentials( SessionId(), opcId );
			LogRead( 𐢜("Unsubscribe: opcId: '{}', user: '{}', nodeCount: {}", opcId, loginName, nodes.size()), requestId );
			auto pClient = ( co_await UAClient::GetClient(move(opcId), loginName, password) ).SP<UAClient>();
			auto [successes,failures] = pClient->MonitoredNodes.Unsubscribe( move(nodes), self );
			Write( FromServer::UnsubscribeTrans(requestId, move(successes), move(failures)) );
		}
		catch( IException& e ){
			WriteException( move(e), requestId );
		}
	}

	α ServerSocketSession::WriteException( IException&& e, RequestId requestId )ι->void{
		LogWriteException( e, requestId );
		Write( FromServer::ExceptionTrans(move(e), requestId) );
	}

	α ServerSocketSession::OnRead( FromClient::Transmission&& transmission )ι->void{
		for( auto i=0; i<transmission.messages_size(); ++i ){
			auto& m = *transmission.mutable_messages( i );
			var requestId = m.request_id();
			switch( m.Value_case() ){
			using enum FromClient::Message::ValueCase;
			case kSessionId:
				SetSessionId( m.session_id(), requestId );
				break;
			case kSubscribe:{
				auto& s = *m.mutable_subscribe();
				Subscribe( move(*s.mutable_opc_id()), NodeId::ToNodes(move(*s.mutable_nodes())), requestId );
				break;}
			case kUnsubscribe:{
				auto& u = *m.mutable_unsubscribe();
				Unsubscribe( move(*u.mutable_opc_id()), NodeId::ToNodes(move(*u.mutable_nodes())), requestId );
				break;}
			default:
				LogRead( 𐢜("Unknown message type '{}'", underlying(m.Value_case())), requestId, ELogLevel::Critical );
			}
		}
	}
}