#include <iostream>
#include "types/OpcServer.h"
#include "uatypes/UAClient.h"
#include "uatypes/Variant.h"
#include "Rest.h"
#include <format>

#define var const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}
namespace Jde::Iot{
	α Test()ι->void;
	int Test2();
}

int main( int argc, char **argv ){
	using namespace Jde;
	auto exitCode = EXIT_SUCCESS;
	try{
		OSApp::Startup( argc, argv, "IotWebSocket", "IOT Connection" );
		DB::CreateSchema();
		DB::SetQLDataSource( DB::DataSourcePtr() );
	}
	catch( const IException& e ){
		std::cout << "Exiting on error:  (" <<e.Code << ")" << e.what() << std::endl;
		exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
	if( !exitCode){
		IApplication::AddShutdownFunction( [](){Iot::UAClient::Shutdown();} );
		Iot::Rest::DoAccept();//todo throw on error... port already in use etc.
		INFOT( AppTag(), "Started IotWebSocket" );
		IApplication::Pause();
	}
	IApplication::Cleanup();
	return EXIT_SUCCESS;
}
