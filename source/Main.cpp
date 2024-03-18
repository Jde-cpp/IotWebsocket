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

int main( int argc, char **argv ){
	using namespace Jde;
	auto exitCode = EXIT_SUCCESS;
#ifdef _MSC_VER
	#ifndef _NDEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	char* placeholder = new char[795];
	std::cout << (uint)placeholder << std::endl;//7286
	std::fill(placeholder, placeholder + 795, '~'); 
	//std::fill(placeholder, placeholder + 795, 'X'); 
	#endif
#endif
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
