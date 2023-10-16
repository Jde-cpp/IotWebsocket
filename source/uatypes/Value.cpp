#include "Value.h"
#include "../uatypes/UAClient.h"

#define var const auto

namespace Jde::Iot{
	namespace Read{
		α Await::await_suspend( HCoroutine h )ι->void{
			IAwait::await_suspend( h );
			_client->SendReadRequest( move(_response), move(_client), move(h) );
		}
		//α Execute( sp<UAClient> ua, NodeId node, Web::Rest::Request req, bool snapShot )ι->Task;

		α OnResponse( UA_Client *ua, void *userdata, UA_UInt32 requestId, UA_StatusCode sc, UA_DataValue *val )ι->void
		{
			var handle = userdata ? (uint)userdata : requestId;
			string logPrefix = format( "[{:x}.{}.{}]", (uint)ua, handle, requestId );
			DBG( "[{}]Value::OnResponse()", logPrefix );
			auto ppClient = Try<sp<UAClient>>( [ua](){return UAClient::Find(ua);} ); if( !ppClient ) return;
			up<flat_map<tuple<uint,uint>, Value>> results;
			bool visited = (*ppClient)->_readRequests.visit( handle, [handle, requestId, sc, val, &results, ua]( auto& pair ){
				auto& x = pair.second;
				if( sc ){
					optional<HCoroutine> h = UAClient::ClearRequest( ua, handle ); if( !h ) return CRITICAL( "Could not find handle for client={:x}, request={}.", (uint)ua, requestId );
					ResumeEx( UAException{sc}, move(*h) );
				}
				else if( auto pRequest=x.Requests.find(requestId); pRequest!=x.Requests.end() ){
					Value v{ *val };
					x.Results.try_emplace( pRequest->second, move(v) );
					if( x.Results.size()==x.Requests.size() )
						results = mu<flat_map<tuple<uint,uint>, Value>>( move(x.Results) );
				}
			});
			if( !visited )
				CRITICAL( "{}Could not find handle.", logPrefix );
			else if( results ){
				optional<HCoroutine> h = UAClient::ClearRequest( ua, handle ); if( !h ) return CRITICAL( "Could not find handle for client={:x}, request={}.", (uint)ua, requestId );
				Resume( move(results), move(*h) );
			}
			DBG( "[{}]Value::~OnResponse()", logPrefix );
		}
	}
#define ADD add.operator()
#define IS(ua) type==&UA_TYPES[ua]
	α Value::ToJson()ε->json{
		if( status )
			return json{ {"sc", status} };
		var scaler = UA_Variant_isScalar( &value );
		var count = scaler ? 1 : value.arrayLength;
		var type = value.type;
		json j{ scaler ? json::object() : json::array() };
		auto add = [scaler, &j]<class T>( void* d, uint i ){ T& x = ((T*)d)[i]; if( scaler ) j = x; else j.push_back(x); };
		auto addExplicit = [scaler, &j]( json&& x ){ if( scaler ) j = x; else j.push_back(x); };
		for( uint i=0; i<count; ++i ){
			if( IS(UA_TYPES_BOOLEAN) )
				ADD<UA_Boolean>( value.data, i );
			else if( IS(UA_TYPES_BYTE) )
				addExplicit( (unsigned char)((UA_Byte*)value.data)[i] );
			else if( IS(UA_TYPES_BYTESTRING) ) [[unlikely]]
				addExplicit( ByteStringToJson(((UA_ByteString*)value.data)[i]) );
			else if( IS(UA_TYPES_DATETIME) )
				addExplicit( UADateTime{((UA_DateTime*)value.data)[i]}.ToJson() );
			else if( IS(UA_TYPES_DOUBLE) )
				ADD<UA_Double>( value.data, i );
			else if( IS(UA_TYPES_DURATION) ) [[unlikely]]
				ADD<UA_Duration>( value.data, i );
			else if( IS(UA_TYPES_EXPANDEDNODEID) ) [[unlikely]]
				addExplicit( Iot::ToJson( ((UA_ExpandedNodeId*)value.data)[i] ) );
			else if( IS(UA_TYPES_FLOAT) )
				ADD<UA_Float>( value.data, i );
			else if( IS(UA_TYPES_GUID) ) [[unlikely]]
				addExplicit( Iot::ToJson(((UA_Guid*)value.data)[i]) );
			else if( IS(UA_TYPES_INT16) ) [[likely]]
				ADD<UA_Int16>( value.data, i );
			else if( IS(UA_TYPES_INT32) ) [[likely]]
				ADD<UA_Int32>( value.data, i );
			else if( IS(UA_TYPES_INT64) )
				addExplicit( Iot::ToJson(((UA_Int64*)value.data)[i]) );
			else if( IS(UA_TYPES_NODEID) )
				addExplicit( Iot::ToJson((((UA_NodeId*)value.data)[i])) );
			else if( IS(UA_TYPES_SBYTE) )
				addExplicit( (char)((UA_SByte*)value.data)[i] );
			else if( IS(UA_TYPES_STATUSCODE) )
				ADD<UA_StatusCode>( value.data, i );
			else if( IS(UA_TYPES_STRING) ) [[likely]]
				addExplicit( ToSV(((UA_String*)value.data)[i]) );
			else if( IS(UA_TYPES_UINT16) )
				ADD<UA_UInt16>( value.data, i );
			else if( IS(UA_TYPES_UINT32) ) [[likely]]
				ADD<UA_UInt32>( value.data, i );
			else if( IS(UA_TYPES_UINT64) )
				addExplicit( Iot::ToJson(((UA_UInt64*)value.data)[i]) );
			else if( IS(UA_TYPES_XMLELEMENT) ) [[unlikely]]
				addExplicit( ToSV(((UA_XmlElement*)value.data)[i]) );
			else{
				WARN( "Unsupported type {}.", type->typeName );
				addExplicit( format("Unsupported type {}.", type->typeName) );
			}
		}
		return j;
/*

		else if( IS(UA_TYPES_BYTESTRING) )
			j = string{, *(UA_Byte
*/
	}

			// auto pIndexes = x.Ids.find( requestId );
				// if( pIndexes==x.Ids.end() ){
				// 	done = true;
				// 	ResumeEx( Exception{"{}Could not find indexes for request.", logPrefix}, move(x.HCo) );
				// }

				//var i = get<0>( pIndexes->second ); var j = get<1>( pIndexes->second );
				// if( i>=x.Response->resultsSize )
				// 	return CRITICAL( "{}Incorrect indexes for request i={}, resultsSize={}.", logPrefix, i, x.Response->resultsSize );
				// if( j>=x.Response->results[i].referencesSize )
				// 	return CRITICAL( "{}Incorrect indexes for request j={}, referencesSize={}.", logPrefix, j, x.Response->results[i].referencesSize );
				// const UA_ReferenceDescription& ref = browse->results[i].references[j];
//				ref.nodeId.nodeId;
//				x.Browse.[x.Ids[requestId].first].references[x.Ids[requestId].second].value = Value{ sc };
			// --x.Count;
			// if( !x.Count )
			// 	ResumeSP( move(x.Response), move(x.Handle) );
		//}
		// UAClient::AddReadResult( handle, requestId, sc ua, var );
		// //optional<HCoroutine> h = UAClient::ClearRequest( client, requestId ); if( !h ) return CRITICAL( "Could not find handle for client={:x}, request={}.", (uint)client, requestId );

    // printf("%-50s%u\n", "Read value attribute for request", requestId);
    // if(UA_Variant_hasScalarType(&var->value, &UA_TYPES[UA_TYPES_INT32])) {
    //     UA_Int32 int_val = *(UA_Int32*) var->value.data;
    //     printf("---%-40s%-8i\n",
    //            "Reading the value of node (1, \"the.answer\"):", int_val);
    // }
}