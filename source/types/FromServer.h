#pragma once

#define $ noexcept->MessageUnion
//#define var const auto
#define SET(x) MessageUnion m; m.x( p ); return m
namespace Jde::Iot::FromServer
{
	Ξ ToAck( uint32 id )${ auto p = new Acknowledgement{}; p->set_id( id ); SET(set_allocated_acknowledgement); }
	Ξ ToCustom( uint32 id, string&& x )${ auto p = new Custom{}; p->set_request_id( id ); p->set_message( move(x) ); SET(set_allocated_custom); }
	Ξ ToError( uint32 id, string&& x )${ auto p = new Error{}; p->set_request_id( id ); p->set_message( move(x) ); SET(set_allocated_error); }
	Ξ ToQueryResult( uint32 id, string&& x )${ auto p = new QueryResult{}; p->set_request_id( id ); p->set_result( move(x) ); SET(set_allocated_query); }
}
#undef $
#undef SET
//#undef var