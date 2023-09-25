#pragma once

namespace Jde::Iot
{
	constexpr UA_String operator "" _uv( const char* x, unsigned long len )noexcept{ return UA_String{ len, (UA_Byte*)x }; }

	struct Logger : UA_Logger
	{
		Logger(){};
		Ω Instance()ι->UA_Logger{ _instance=mu<Logger>(); _instance->context=&_context; return *_instance; }
		Ω UA_Log_Stdout_log( void *context, UA_LogLevel level, UA_LogCategory category, const char* file, const char* function, uint_least32_t line, const char *msg, va_list args )ι->void;
		Ω clear( void *context )ι->void{ }

	private:
		Ω Format( const char* format, va_list ap )ι->string;
		static int _context; /* Logger state */
		static up<Logger> _instance;
	};
}