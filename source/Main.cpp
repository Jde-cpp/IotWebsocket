#include <iostream>
#include <jde/iot/uatypes/UAClient.h>
#include "Rest.h"
#include "Socket.h"
#include <format>

#define var const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}

int main( int argc, char **argv ){
	using namespace Jde;
	auto exitCode = EXIT_SUCCESS;
	sv _ = "main";
	try{
		OSApp::Startup( argc, argv, "Jde.IotWebSocket", "IOT Connection" );
		DB::CreateSchema();
		DB::SetQLDataSource( DB::DataSourcePtr() );
	}
	catch( const IException& e ){
		std::cout << (e.Level()==ELogLevel::Trace ? "Exiting:  " : "Exiting on error:  ") <<  '(' << std::hex << e.Code << ')' << e.what() << std::endl;
		exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
	if( !exitCode){
		IApplication::AddShutdownFunction( [](){Iot::UAClient::Shutdown();} );
		Iot::Rest::Start();//todo throw on error... port already in use etc.
		Iot::Socket::Start();
		INFOT( AppTag(), "Started IotWebSocket" );
		IApplication::Pause();
	}
	IApplication::Cleanup();
	return EXIT_SUCCESS;
}
