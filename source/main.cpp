#include <iostream>
#include <jde/framework/thread/execution.h>
#include <jde/access/access.h>
#include <jde/db/db.h>
#include <jde/ql/ql.h>
#include <jde/app/client/appClient.h>
#include <jde/app/client/AppClientSocketSession.h>
#include <jde/opc/exports.h>
#include <jde/opc/opc.h>
#include <jde/opc/OpcQLHook.h>
#include <jde/opc/uatypes/UAClient.h>
#include "WebServer.h"

#define let const auto
namespace Jde{
	α OSApp::ProductName()ι->sv{ return "OpcGateway"; }
	Ω startUp( int argc, char **argv )ι->Access::ConfigureAwait::Task;
	optional<int> _exitCode;
}

α main( int argc, char **argv )->int{
	using namespace Jde;
	startUp( argc, argv );
	_exitCode = IApplication::Pause();
	Process::Shutdown( _exitCode.value_or(EXIT_FAILURE) );
	return _exitCode.value_or( EXIT_FAILURE );
}

α Jde::startUp( int argc, char **argv )ι->Access::ConfigureAwait::Task{
	try{
		TagParser( Opc::LogTagParser );
		OSApp::Startup( argc, argv, "Jde.OpcGateway", "IOT Connection" );
		auto authorize = App::Client::RemoteAcl();
		auto schema = DB::GetAppSchema( "opc", authorize );
		QL::Configure( {schema} );
		Opc::Configure( schema );
		if( auto sync = Settings::FindBool("/dbServers/sync").value_or(true); sync )
			DB::SyncSchema( *schema, QL::Local() );

		Crypto::CryptoSettings settings{ "http/ssl" };
		if( !fs::exists(settings.PrivateKeyPath) ){
			settings.CreateDirectories();
			Crypto::CreateKeyCertificate( settings );
		}

		App::Client::Connect();
		Execution::Run();
		while( App::Client::AppClientSocketSession::Instance()==nullptr || App::Client::QLServer()==nullptr )
			std::this_thread::yield();
		auto await = Access::Configure( DB::GetAppSchema("access", authorize), {schema}, App::Client::QLServer(), UserPK{UserPK::System} );
		co_await await;
		Process::AddShutdownFunction( [](bool terminate){Opc::UAClient::Shutdown(terminate);} );
		QL::Hook::Add( mu<Opc::OpcQLHook>() );
		Opc::StartWebServer();
		Information( ELogTags::App, "---Started {}---", OSApp::ProductName() );
	}
	catch( const IException& e ){
		Critical( ELogTags::App, "Exiting on error:  {}"sv, e.what() );
		_exitCode = e.Code ? (int)e.Code : EXIT_FAILURE;
	}
}