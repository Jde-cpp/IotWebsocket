#include <iostream>
#include <jde/log/Log.h>
#include <jde/db/graphQL/GraphQLHook.h>
#include <jde/iot/Exports.h>
#include <jde/iot/uatypes/UAClient.h>
#include <jde/iot/IotGraphQL.h>
#include "Server.h"
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
		DB::GraphQL::Hook::Add( mu<Iot::IotGraphQL>() );
		try{
			Iot::StartWebServer();
			INFOT( AppTag(), "---Started IotWebSocket---" );
			exitCode = IApplication::Pause();
		}
		catch( const IException& e ){
			Critical( ToLogTags(AppTag()->Id), "Exiting on error:  {}"sv, e.what() );
			exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
		}
	}
	IApplication::Shutdown( exitCode.value_or(EXIT_FAILURE) );
	IApplication::Cleanup();
	return exitCode.value_or( EXIT_FAILURE );
}
