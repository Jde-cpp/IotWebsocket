#include "Node.h"
#define var const auto
namespace Jde::Iot
{
	NodeId::NodeId ( const NodeId& x )ι:
		UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{
		nodeId = x.Copy();
		namespaceUri = UA_String_fromChars( string{ToSV(x.namespaceUri)}.c_str() );
		serverIndex = x.serverIndex;
	}

	NodeId::NodeId( const flat_map<string,string>& x )ι:
		UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{
		try{
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
				ToGuid( p->second, nodeId.identifier.guid );
			}
			else
				throw Exception( "No identifier in nodeId" );
		}
		catch( json::exception& e ){
			throw Exception( SRCE_CUR, move(e) );
		}
	}

	NodeId::NodeId( const json& j )ε:
		UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}{
		if( j.find("nsu")!=j.end() )
			namespaceUri = UA_String_fromChars( j["nsu"].get<string>().c_str() );
		if( j.find("serverindex")!=j.end() )
			j.at("serverindex").get_to( serverIndex );
		if( j.find("ns")!=j.end() )
			j.at("ns").get_to( nodeId.namespaceIndex );
		if( j.find("s")!=j.end() ){
			nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_STRING;
			nodeId.identifier.string = UA_String_fromChars( j["s"].get<string>().c_str() );
		}
		else if( j.find("i")!=j.end() ){
			nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_NUMERIC;
			j.at("i").get_to( nodeId.identifier.numeric );
		}
		else if( j.find("b")!=j.end() ){
			nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING;
			var v = ToUV( j["b"].get<string>() );
			UA_ByteString_fromBase64( &nodeId.identifier.byteString, &v );
		}
		else if( j.find("g")!=j.end() ){
			nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_GUID;
			ToGuid( j["g"].get<string>(), nodeId.identifier.guid );
		}
	}

	NodeId::NodeId( NodeId&& x )ι:
		UA_ExpandedNodeId{UA_EXPANDEDNODEID_NULL}
	{
		nodeId = x.Move();
		namespaceUri = x.namespaceUri;
		serverIndex = x.serverIndex;
		memset( &x, 0, sizeof(UA_ExpandedNodeId) );
	}

	NodeId::~NodeId()
	{
		if( namespaceUri.length )
			UA_String_clear( &namespaceUri );
		if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_STRING && nodeId.identifier.string.length )
			UA_String_clear( &nodeId.identifier.string );
		else if( nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING  && nodeId.identifier.byteString.length )
			UA_ByteString_clear( &nodeId.identifier.byteString );
	}

	α NodeId::operator<( const NodeId& x )Ι->bool{
		return
			ToSV(namespaceUri)==ToSV(x.namespaceUri) ?
				serverIndex==x.serverIndex ?
					nodeId.namespaceIndex==x.nodeId.namespaceIndex ?
						nodeId.identifierType==x.nodeId.identifierType ?
							nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_NUMERIC ? nodeId.identifier.numeric<x.nodeId.identifier.numeric :
								nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_STRING ? ToSV(nodeId.identifier.string)<ToSV(x.nodeId.identifier.string) :
								nodeId.identifierType==UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING ? ToSV(nodeId.identifier.byteString)<ToSV(x.nodeId.identifier.byteString)
							: memcmp( &nodeId.identifier.guid, &x.nodeId.identifier.guid, sizeof(UA_Guid) )<0
						: nodeId.identifierType<x.nodeId.identifierType
					: nodeId.namespaceIndex<x.nodeId.namespaceIndex
				: serverIndex<x.serverIndex
			: ToSV(namespaceUri)<ToSV(x.namespaceUri);
	}

	α NodeId::Copy()Ι->UA_NodeId{
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

	α NodeId::Move()ι->UA_NodeId{
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
		return y;
	}

	α to_json( nlohmann::json& n, const UA_NodeId& nodeId )ε->void
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
	α ToJson( const UA_NodeId& nodeId )ε->json{
		json j;
		to_json( j, nodeId );
		return j;
	}

	α ToJson( const UA_ExpandedNodeId& x )ε->json{
		json j;
		if( x.namespaceUri.length )
			j["nsu"] = ToSV(x.namespaceUri);
		if( x.serverIndex )
			j["serverindex"] = x.serverIndex;
		to_json( j, x.nodeId );
		return j;
	}
	α NodeId::ToJson()Ε->nlohmann::json{
		return Iot::ToJson( *this );
	}
}