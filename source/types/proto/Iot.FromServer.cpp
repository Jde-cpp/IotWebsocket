#include "Iot.FromServer.h"
#include <jde/iot/uatypes/Node.h>

#define var const auto

namespace Jde::Iot{
	α FromServer::AckTrans( uint32 socketSessionId )ι->FromServer::Transmission{
		FromServer::Message m;
		m.set_ack( socketSessionId );
		return MessageTrans( move(m), 0 );
	}

	α FromServer::CompleteTrans( RequestId requestId )ι->FromServer::Transmission{
		FromServer::Message m;
		return MessageTrans( move(m), requestId );
	}
	α FromServer::ExceptionTrans( const IException& e, optional<RequestId> requestId )ι->FromServer::Transmission{
		FromServer::Transmission t;
		auto& m = *t.add_messages();
		if( requestId )
			m.set_request_id( *requestId );

		auto& proto = *m.mutable_exception();
		proto.set_what( e.what() );
		proto.set_code( e.Code );
		return t;
	}
	α FromServer::MessageTrans( FromServer::Message&& m, RequestId requestId )ι->FromServer::Transmission{
		FromServer::Transmission t;
		m.set_request_id( requestId );
		*t.add_messages() = move(m);
		return t;
	}
	α FromServer::SubscribeAckTrans( up<FromServer::SubscriptionAck>&& ack, RequestId requestId )ι->FromServer::Transmission{
		FromServer::Transmission t;
		auto& m = *t.add_messages();
		m.set_request_id( requestId );
		m.set_allocated_subscription_ack( ack.release() );
		return t;
	}
	α FromServer::UnsubscribeTrans( uint32 id, flat_set<NodeId>&& successes, flat_set<NodeId>&& failures )ι->FromServer::Transmission{
		FromServer::Transmission t;
		auto& m = *t.add_messages();
		m.set_request_id( id );
		auto ack = m.mutable_unsubscribe_ack();
		for_each( move(successes), [&ack](var& n){*ack->add_successes() = n.ToProto();} );
		for_each( move(failures), [&ack](var& n){*ack->add_failures() = n.ToProto();} );
		return t;
	}
}