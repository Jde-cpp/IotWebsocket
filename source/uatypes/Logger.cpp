#include "Logger.h"

#define var const auto
namespace Jde::Iot
{
	int Logger::_context={}; /* Logger state */
	up<Logger> Logger::_instance={};

	α Logger::UA_Log_Stdout_log( void *context, UA_LogLevel level, UA_LogCategory category, const char* file, const char* function, uint_least32_t line, const char *msg, va_list args )ι->void
	{
		constexpr array<sv,8> Categories{ "UANet", "UASecure", "UASession", "UAServer", "UAClient", "UAUser", "UASecurity", "UAEvent" };
		var tag = Str::FromEnum( Categories, category );
		auto tagLevel = Logging::TagLevel(Str::FromEnum(Categories,category)).Level;
		tagLevel = ELogLevel::Trace;//
		if( (int)tagLevel<=(int)level )
		{
/*				string m2{msg};
			va_list ap_copy; va_copy( ap_copy, args );
			var len = vsnprintf( 0, 0, msg, args );
			string m; m.resize( len + 1 );
			vsnprintf( m.data(), len + 1, msg, args );
			m.resize( len );  // remove the NUL*/
			ELogLevel myLevel = (ELogLevel)((int)level/100-1);
			Logging::Log( Logging::Message(tag, (ELogLevel)myLevel, Format(msg,args), file, function, line), false );
		}
	}

	α Logger::Format( const char* format, va_list ap )ι->string
	{
		va_list ap_copy; va_copy( ap_copy, ap );
		var len = vsnprintf( 0, 0, format, ap_copy );
		string m; m.resize( len + 1 );
		vsnprintf( m.data(), len + 1, format, ap );
		m.resize( len );  // remove the NUL
		return m;
	}

}