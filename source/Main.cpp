#include <iostream>
#include <jde/Log.h>
#include <jde/App.h>
#include <jde/Str.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
//$(VC_IncludePath);$(WindowsSDK_IncludePath);

#define var const auto
namespace Jde
{
	α OSApp::ProductName()ι->sv{ return "IotWebsocket"; }
}
namespace Jde::Iot
{
	constexpr UA_String operator "" _uv( const char* x, size_t len )noexcept{ return UA_String{ len, (UA_Byte*)x }; }

	struct Logger : UA_Logger
	{
		Ω UA_Log_Stdout_log( void *context, UA_LogLevel level, UA_LogCategory category, const char* file, const char* function, uint_least32_t line, const char *msg, va_list args )->void
		{
			constexpr array<sv,8> Categories{ "UANet", "UASecure", "UASession", "UAServer", "UAClient", "UAUser", "UASecurity", "UAEvent" };
			var tag = Str::FromEnum( Categories, category );
			auto tagLevel = Logging::TagLevel(Str::FromEnum(Categories,category)).Level;
			tagLevel = ELogLevel::Trace;//
			if( (int)tagLevel<=(int)level )
			{
				string m2{msg};
				va_list ap_copy; va_copy( ap_copy, args );
				var len = vsnprintf( 0, 0, msg, args );
				string m; m.resize( len + 1 );
				vsnprintf( m.data(), len + 1, msg, args );
				m.resize( len );  // remove the NUL
				Logging::Log( Logging::Message(tag, (ELogLevel)level, Format(msg,args), file, function, line), false );
			}
		}
		Ω clear( void *context )->void
		{
			std::cout << "clear" << std::endl;
		}

	private:
		Ω Format( const char* format, va_list ap )->string
		{
			va_list ap_copy; va_copy( ap_copy, ap );
			var len = vsnprintf( 0, 0, format, ap_copy );
			string m; m.resize( len + 1 );
			vsnprintf( m.data(), len + 1, format, ap );
			m.resize( len );  // remove the NUL
			return m;
		}
		void *context{}; /* Logger state */

	};

	struct UAClient
	{
		UAClient():_ptr{ Create() }
		{
			auto& c = Configuration();
			/*        UA_Logger logger = {Logger::UA_Log_Stdout_log, (void*)context, UA_Log_Stdout_clear};
			return logger;

			c.logger = Logger{};
			c.logger.clear( nullptr );*/
			UA_ClientConfig_setDefault( &c );
		}
		~UAClient(){ UA_Client_delete(_ptr); }
		auto Create()->UA_Client* //ua_config_default.c
		{
			//      memset( &_config, 0, sizeof(UA_ClientConfig) );
			//UA_Logger logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
			_config.logger = { Logger::UA_Log_Stdout_log, (void*)_logContext, Logger::clear };
			//UA_Log_Stdout_withLevel(UA_LOGLEVEL_INFO);
			_config.eventLoop = UA_EventLoop_new_POSIX(&_config.logger);
			UA_ConnectionManager *tcpCM = UA_ConnectionManager_new_POSIX_TCP( "tcp connection manager"_uv );
			_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)tcpCM);

			UA_ConnectionManager *udpCM = UA_ConnectionManager_new_POSIX_UDP( "udp connection manager"_uv );
			_config.eventLoop->registerEventSource(_config.eventLoop, (UA_EventSource *)udpCM);
			UA_Client *c = UA_Client_newWithConfig(&_config);
			UA_Client_getConfig(c)->eventLoop->logger = &_config.logger;

			return c;
		}
		operator UA_Client* (){ return _ptr; }
		void Connect( const char* endpoint )
		{
			if( UA_StatusCode retval = UA_Client_connect( _ptr, endpoint ); retval != UA_STATUSCODE_GOOD )
				throw (int)retval;
		}
		auto Configuration()->UA_ClientConfig&{ return *UA_Client_getConfig(_ptr); }
	private:
		UA_ClientConfig _config{};
		void* _logContext{};
		UA_Client* _ptr;//needs to be after _logContext & _config.
	};
	struct UAVariant
	{
		UA_Variant _value{};
		operator UA_Variant&(){return _value;}
		auto operator &()->UA_Variant*{return &_value;}
		auto data()->void*{ return _value.data; }
	};
}

int main( int argc, char **argv )
{
	using namespace Jde;
	try
	{
		OSApp::Startup( argc, argv, "Connection", "IOT Connection" );
		try
		{
			UM::Configure();
		}
		catch( IException& e )
		{
			std::cerr << e.what() << std::endl;
			{auto e2=e.Move();}//destructor log.
			std::this_thread::sleep_for( 1s );
			std::terminate();
		}

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
		UAClient client;
		//UA_Client *client = UA_Client_new();
		//UA_ClientConfig_setDefault( UA_Client_getConfig(client) );

		client.Connect( "opc.tcp://127.0.0.1:49320" );
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
		/* Clean up */
		UA_Variant_clear(&value);
	}
}