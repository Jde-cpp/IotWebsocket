#include <iostream>
#include "types/OpcServer.h"
#include "uatypes/UAClient.h"
#include "uatypes/UAVariant.h"
#include "Rest.h"

#define var const auto
namespace Jde
{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}
namespace Jde::Iot
{
	α Test()ι->void;
	int Test2();
}

int main( int argc, char **argv )
{
	using namespace Jde;
	try
	{
		OSApp::Startup( argc, argv, "IotWebSocket", "IOT Connection" );
		DB::CreateSchema();
		DB::SetQLDataSource( DB::DataSourcePtr() );
		Iot::Rest::DoAccept();
		//Iot::Test2();
		//Iot::Test();
/*		try
		{
			UM::Configure();
		}
		catch( IException& e )
		{
			std::cerr << e.what() << std::endl;
			{auto e2=e.Move();}//destructor log.
			std::this_thread::sleep_for( 1s );
			std::terminate();
		}*/

		IApplication::Pause();
	}
	catch( const IException& e )
	{
		std::cout << "Exiting on error:  " << e.what() << std::endl;
	}
	IApplication::Cleanup();
	return EXIT_SUCCESS;

}
#define var const auto

namespace Jde::Iot
{
	α Test()ι->void
	{
		UAClient client{ "opc.tcp://127.0.0.1:4840" };
		/* Read the value attribute of the node. UA_Client_readValueAttribute is a wrapper for the raw read service available as UA_Client_Service_read. */
		UAVariant value;
		/* NodeId of the variable holding the current time */
		const UA_NodeId nodeId = UA_NODEID_NUMERIC( 0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME );
		UA_StatusCode retval = UA_Client_readValueAttribute(client, nodeId, &value);
		if( retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME]) )
		{
			UA_DateTime raw_date = *(UA_DateTime *)value.data();
			UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
			//Jde::Logging::Log( Jde::Logging::MessageBase("date is: {}"sv, Jde::ELogLevel::Debug, __FILE__, __func__, __LINE__), dts.day );
			//DBG( "date is: {}", dts.day );
			UA_LOG_INFO( UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "date is: %u-%u-%u %u:%u:%u.%03u\n", dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec );
		}
		char theAnswer []= "the.answer";
		char* pTheAnswer = theAnswer;
    var status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, pTheAnswer), &value);
    if(status == UA_STATUSCODE_GOOD &&
       UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        DBG("the value is: {}", *(UA_Int32*)value.data());
    }

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
                printf("%-9u %-16u %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
                       ref->browseName.name.data, (int)ref->displayName.text.length,
                       ref->displayName.text.data);
            } else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                printf("%-9u %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       (int)ref->nodeId.nodeId.identifier.string.length,
                       ref->nodeId.nodeId.identifier.string.data,
                       (int)ref->browseName.name.length, ref->browseName.name.data,
                       (int)ref->displayName.text.length, ref->displayName.text.data);
            }
            /* TODO: distinguish further types */
        }
    }
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);

		/* Clean up */
		UA_Variant_clear(&value);
	}
	int Test2()
	{
  UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if(status != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return status;
    }

    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
     * wrapper for the raw read service available as UA_Client_Service_read. */
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Variant_init(&value);
		char theAnswer []= "the.answer";
		char* pTheAnswer = theAnswer;
    status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, pTheAnswer), &value);
    if(status == UA_STATUSCODE_GOOD &&
       UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        printf("the value is: %i\n", *(UA_Int32*)value.data);
    }

    /* Clean up */
    UA_Variant_clear(&value);
    UA_Client_delete(client); /* Disconnects the client internally */
    return status == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
	}
}