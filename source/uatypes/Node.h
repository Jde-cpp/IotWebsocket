#pragma once
#include "helpers.h"

namespace Jde::Iot
{
	struct NodeId : UA_ExpandedNodeId{
//		NodeId( const UA_ExpandedNodeId& x )ι{ UA_ExpandedNodeId_copy( &x, this ); }
//		NodeId( const UA_NodeId& x )ι{ UA_NodeId_copy( &x, this ); namespaceUri=""_ua, serverIndex=0; }
		NodeId( UA_NodeId&& x )ι:UA_ExpandedNodeId{move(x), ""_uv, 0}{}
		NodeId( const flat_map<string,string>& x )ι:
			UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{
			if( auto p = x.find("nsu"); p!=x.end() )
				namespaceUri = UA_String_fromChars( p->second.c_str() );
			if( auto p = x.find("serverindex"); p!=x.end() )
				serverIndex = stoul( p->second );
			if( auto p = x.find("ns"); p!=x.end() )
				nodeId.namespaceIndex = stoul( p->second );
			if( auto p = x.find("s"); p!=x.end() ){
				nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_STRING;
				nodeId.identifier.string = UA_String_fromChars( p->second.c_str() );
			}
			else if( auto p = x.find("i"); p!=x.end() ){
				nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_NUMERIC;
				nodeId.identifier.numeric = stoul( p->second );
			}
			else if( auto p = x.find("b"); p!=x.end() ){
				nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING;
				auto t = ToUV( p->second );
				UA_ByteString_fromBase64( &nodeId.identifier.byteString, &t );
			}
			else if( auto p = x.find("g"); p!=x.end() ){
				nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_GUID;
				string stringGuid = p->second;
				std::erase( stringGuid, '-' );
        auto uuid = boost::lexical_cast<boost::uuids::uuid>( stringGuid );
				memcpy( &nodeId.identifier.guid, &uuid.data, sizeof(nodeId.identifier) );
			}
		}
		NodeId ( NodeId&& x )ι:
			UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}
		{
			nodeId = x.Move();
			namespaceUri = x.namespaceUri;
			serverIndex = x.serverIndex;
			memset( &x, 0, sizeof(UA_ExpandedNodeId) );
		}
		α operator=( NodeId&& x )ι->NodeId&{
			nodeId = x.Move();
			namespaceUri=x.namespaceUri;
			serverIndex=x.serverIndex;
			memset( &x, 0, sizeof(UA_ExpandedNodeId) );
			return *this;
		}

		~NodeId()
		{
			if( namespaceUri.length )
				UA_String_clear( &namespaceUri );
			if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_STRING && nodeId.identifier.string.length )
				UA_String_clear( &nodeId.identifier.string );
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING  && nodeId.identifier.byteString.length )
				UA_ByteString_clear( &nodeId.identifier.byteString );
		}
		α Copy()ι->UA_NodeId{
			UA_NodeId y{};
	    y.namespaceIndex = nodeId.namespaceIndex;
	    y.identifierType = nodeId.identifierType;
			if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_NUMERIC )
				y.identifier.numeric = nodeId.identifier.numeric;
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_STRING )
				y.identifier.string = UA_String_fromChars( string{ToSV(nodeId.identifier.string)}.c_str() );
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING )
				y.identifier.byteString =  UA_BYTESTRING_ALLOC( string{ToSV(nodeId.identifier.byteString)}.c_str() );
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_GUID )
				y.identifier.guid = nodeId.identifier.guid;
			return y;
		}
		α Move()ι->UA_NodeId{
			UA_NodeId y{};
	    y.namespaceIndex = nodeId.namespaceIndex;
	    y.identifierType = nodeId.identifierType;
			if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_NUMERIC )
				y.identifier.numeric = nodeId.identifier.numeric;
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_STRING )
		    y.identifier.string = nodeId.identifier.string;
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING )
				y.identifier.byteString = nodeId.identifier.byteString;
			else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_GUID )
				y.identifier.guid = nodeId.identifier.guid;
			memset( &nodeId, 0, sizeof(UA_NodeId) );
			//UA_ExpandedNodeId_clear( this );
			return y;
		}
	};

	Ξ to_json( nlohmann::json& n, const UA_NodeId& nodeId )ε->void
	{
		const UA_NodeIdType type = nodeId.identifierType;
		//n["identifierType"] = type;
		n["ns"] = nodeId.namespaceIndex;
		if( type==UA_NodeIdType::UA_NODEIDTYPE_NUMERIC )
			n["i"]=nodeId.identifier.numeric;
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_STRING )
			n["s"]=ToSV( nodeId.identifier.string );
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_GUID )
			n["g"] = ToJson( nodeId.identifier.guid );
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING )
			n["b"] = ByteStringToJson( nodeId.identifier.byteString );
	}
	Ξ ToJson( const UA_NodeId& n )ε->nlohmann::json{
		nlohmann::json j;
		to_json( j, n );
		return j;
	}
	Ξ ToJson( const UA_ExpandedNodeId& n )ε->nlohmann::json{
		nlohmann::json j;
		to_json( j, n.nodeId );
		j["nsu"] = ToSV(n.namespaceUri);
		j["serverindex"] = n.serverIndex;

		return j;
	}
}