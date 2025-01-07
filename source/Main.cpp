#include <iostream>
#include <jde/access/access.h>
#include <jde/app/client/appClient.h>
#include <jde/db/db.h>
#include <jde/ql/ql.h>
#include <jde/opc/exports.h>
#include <jde/opc/OpcQLHook.h>
#include <jde/opc/uatypes/UAClient.h>
#include "WebServer.h"

#define let const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
	Ω startUp( int argc, char **argv )ι->Access::ConfigureAwait::Task;
	optional<int> _exitCode;
}

α main( int argc, char **argv )->int{
	using namespace Jde;
	startUp( argc, argv );
	Process::Shutdown( _exitCode.value_or(EXIT_FAILURE) );
	return _exitCode.value_or( EXIT_FAILURE );
}

α Jde::startUp( int argc, char **argv )ι->Access::ConfigureAwait::Task{
	try{
		TagParser( Opc::LogTagParser );
		OSApp::Startup( argc, argv, "Jde.IotWebSocket", "IOT Connection" );
		auto authorize = App::Client::RemoteAcl();
		auto schema = DB::GetAppSchema( "opc", authorize );
		QL::Configure( {schema} );
		auto await = Access::Configure( DB::GetAppSchema("access", authorize), {schema}, QL::Local(), UserPK{UserPK::System} );
		co_await await;
		Process::AddShutdownFunction( [](bool terminate){Opc::UAClient::Shutdown(terminate);} );
		QL::Hook::Add( mu<Opc::OpcQLHook>() );
		App::Client::Connect();
		Opc::StartWebServer();
		Information( ELogTags::App, "---Started IotWebSocket---" );
		_exitCode = IApplication::Pause();
	}
	catch( const IException& e ){
		Critical( ELogTags::App, "Exiting on error:  {}"sv, e.what() );
		_exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
}