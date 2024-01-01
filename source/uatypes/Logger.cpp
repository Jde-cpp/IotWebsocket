#include "Logger.h"

#define var const auto
namespace Jde::Iot
{
	int Logger::_context={}; /* Logger state */
	up<Logger> Logger::_instance={};

	α Logger::UA_Log_Stdout_log( void *context, UA_LogLevel uaLevel, UA_LogCategory category, const char* file, const char* function, uint_least32_t line, const char *msg, va_list args )ι->void{
		constexpr array<sv,8> Categories{ "UANet", "UASecure", "UASession", "UAServer", "UAClient", "UAUser", "UASecurity", "UAEvent" };
		var level = (ELogLevel)( (int)uaLevel/100-1 ); //level==UA_LOGLEVEL_DEBUG=200
		var tag = Str::FromEnum( Categories, category );
		var categoryLevel = Logging::TagLevel(tag).Level;//myLevel==ELogLevel::Debug=1
		if( (int)categoryLevel>=(int)level )
			Logging::Log( Logging::Message(tag, level, Format(msg,args), file, function, line), false );
	}

	α Logger::Format( const char* format, va_list ap )ι->string{
		va_list ap_copy; va_copy( ap_copy, ap );
		var len = vsnprintf( 0, 0, format, ap_copy );
		string m; m.resize( len + 1 );
		vsnprintf( m.data(), len + 1, format, ap );
		m.resize( len );  // remove the NUL
		return m;
	}
}