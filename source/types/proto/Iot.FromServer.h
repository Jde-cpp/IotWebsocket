#pragma once
namespace Jde::Iot{ struct NodeId; }
namespace Jde::Iot::FromServer{
	α AckTrans( uint socketSessionId )ι->FromServer::Transmission;
	α CompleteTrans( RequestId requestId )ι->FromServer::Transmission;
	α SubscribeAckTrans( up<FromServer::SubscriptionAck>&& ack, RequestId requestId )ι->FromServer::Transmission;
	α UnsubscribeTrans( uint32 id, flat_set<NodeId>&& successes, flat_set<NodeId>&& failures )ι->FromServer::Transmission;
	α MessageTrans( FromServer::Message&& m, RequestId requestId )ι->FromServer::Transmission;
	α ExceptionTrans( const IException& e, optional<RequestId> requestId )ι->FromServer::Transmission;
	// struct Acknowledgement;
	// struct Exception;
	// struct UnsubscribeResult;
	// α ToAck( uint32 id )ι->Acknowledgement;
	// α ToException( uint32 id, string&& x )ι->Exception;

}