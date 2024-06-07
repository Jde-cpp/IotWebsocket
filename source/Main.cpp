#include <iostream>
#include <jde/Log.h>
#include <jde/db/graphQL/GraphQLHook.h>
#include <jde/iot/Exports.h>
#include <jde/iot/uatypes/UAClient.h>
#include <jde/iot/IotGraphQL.h>
#include "../../Public/src/web/Exports.h"
#include "../../Public/src/web/Exports.h"
#include "Rest.h"
#include "Socket.h"
#include <format>

#define var const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}

int main( int argc, char **argv ){
	using namespace Jde;
	optional<int> exitCode;
	try{
		OSApp::Startup( argc, argv, "Jde.IotWebSocket", "IOT Connection" );
		DB::CreateSchema();
		DB::SetQLDataSource( DB::DataSourcePtr() );
	}
	catch( const IException& e ){
		std::cout << (e.Level()==ELogLevel::Trace ? "Exiting:  " : "Exiting on error:  ") <<  '(' << std::hex << e.Code << ')' << e.what() << std::endl;
		exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
	if( !exitCode ){
		IApplication::AddShutdownFunction( [](){Iot::UAClient::Shutdown();} );
		Iot::Rest::Start();//TODO throw on error... port already in use etc.
		Iot::Socket::Start();
		DB::GraphQL::Hook::Add( mu<Iot::IotGraphQL>() );
		INFOT( AppTag(), "Started IotWebSocket" );
		exitCode = IApplication::Pause();
	}
	IApplication::Shutdown( exitCode.value_or(EXIT_FAILURE) );
	IApplication::Cleanup();
	return exitCode.value_or( EXIT_FAILURE );
}
