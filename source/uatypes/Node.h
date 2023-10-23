#pragma once
#include "helpers.h"

namespace Jde::Iot
{
	struct NodeId : UA_ExpandedNodeId{
//		NodeId( const UA_ExpandedNodeId& x )ι{ UA_ExpandedNodeId_copy( &x, this ); }
//		NodeId( const UA_NodeId& x )ι{ UA_NodeId_copy( &x, this ); namespaceUri=""_ua, serverIndex=0; }
		NodeId( UA_NodeId&& x )ι:UA_ExpandedNodeId{move(x), ""_uv, 0}{}
		NodeId( const UA_ExpandedNodeId& x )ι:UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{ UA_ExpandedNodeId_copy( &x, this ); }
		//NodeId( const UA_ExpandedNodeId&& x )ι:UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{ UA_ExpandedNodeId_copy( &x, this ); }
		NodeId( const flat_map<string,string>& x )ι;
		NodeId( const json& j )ε;
		NodeId( const NodeId& x )ι;
		NodeId( NodeId&& x )ι;
		α operator=( NodeId&& x )ι->NodeId&{
			nodeId = x.Move();
			namespaceUri=x.namespaceUri;
			serverIndex=x.serverIndex;
			memset( &x, 0, sizeof(UA_ExpandedNodeId) );
			return *this;
		}

		~NodeId();
		α operator<( const NodeId& x )Ι->bool;
		α Copy()Ι->UA_NodeId;
		α Move()ι->UA_NodeId;
		α ToJson()Ε->json;
	};

	α ToJson( const UA_NodeId& nodeId )ε->json;
	α ToJson( const UA_ExpandedNodeId& nodeId )ε->json;
}