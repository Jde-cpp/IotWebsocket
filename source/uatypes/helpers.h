#pragma once
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/hex.hpp>

namespace Jde::Iot
{
	Ξ ToSV( const UA_String& s )ι->sv{ return sv{ (const char*)s.data, s.length }; }

	Ξ to_json( nlohmann::json& n, const UA_NodeId& nodeId )ε->void
	{
		const UA_NodeIdType type = nodeId.identifierType;
		n["identifierType"] = type;
		n["nsIndex"] = nodeId.namespaceIndex;
		if( type==UA_NodeIdType::UA_NODEIDTYPE_NUMERIC )
			n["value"]=nodeId.identifier.numeric;
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_STRING )
			n["value"]=ToSV( nodeId.identifier.string );
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_GUID ){
			boost::uuids::uuid id;
			memcpy(&id.data, &nodeId.identifier.guid, id.size() );
			n["value"] = boost::uuids::to_string( id );
		}
		else if( type==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING ){
			UA_ByteString bs = nodeId.identifier.byteString;
			string buffer; buffer.reserve( bs.length*2 );
			boost::algorithm::hex_lower( ToSV(bs), std::back_inserter(buffer) );
			n["value"] = buffer;
		}
	}
	Ξ ToJson( const UA_NodeId& nodeId )ε->nlohmann::json{
		nlohmann::json n;
		to_json( n, nodeId );
		return n;
	}
	Ξ ToJson( const UA_ExpandedNodeId& expanded )ε->nlohmann::json{
		nlohmann::json n;
		to_json( n, expanded.nodeId );
		n["ns"] = ToSV(expanded.namespaceUri);
		n["serverIndex"] = expanded.serverIndex;

		return n;
	}

}