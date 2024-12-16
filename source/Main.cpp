#include <iostream>
#include <jde/log/Log.h>
#include <jde/app/client/AppClient.h>
#include <jde/db/graphQL/GraphQLHook.h>
#include <jde/opc/Exports.h>
#include <jde/opc/uatypes/UAClient.h>
#include <jde/opc/IotGraphQL.h>
#include "Server.h"
#include <format>

#define let const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}

int main( int argc, char **argv ){
	using namespace Jde;
	optional<int> exitCode;
	try{
		TagParser( Opc::LogTagParser );
		OSApp::Startup( argc, argv, "Jde.IotWebSocket", "IOT Connection" );
		DB::CreateSchema();
		DB::SetQLDataSource( DB::DataSourcePtr() );
	}
	catch( const IException& e ){
		std::cout << (e.Level()==ELogLevel::Trace ? "Exiting:  " : "Exiting on error:  ") <<  '(' << std::hex << e.Code << ')' << e.what() << std::endl;
		exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
	if( !exitCode ){
		Process::AddShutdownFunction( [](bool terminate){Opc::UAClient::Shutdown(terminate);} );
		DB::GraphQL::Hook::Add( mu<Opc::IotGraphQL>() );
		try{
			App::Client::Connect();
			Opc::StartWebServer();
			Information( ELogTags::App, "---Started IotWebSocket---" );
			exitCode = IApplication::Pause();
		}
		catch( const IException& e ){
			Critical( ELogTags::App, "Exiting on error:  {}"sv, e.what() );
			exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
		}
	}
	Process::Shutdown( exitCode.value_or(EXIT_FAILURE) );
	return exitCode.value_or( EXIT_FAILURE );
}
