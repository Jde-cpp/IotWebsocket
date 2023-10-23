#pragma once

#define UAε(x) var sc = x; if( sc ) throw UAException{ sc, ELogLevel::Error };

namespace Jde::Iot
{
	struct UAException : Exception
	{
		UAException( UA_StatusCode sc, ELogLevel level=ELogLevel::Debug, SRCE )ι:Exception{ Message(sc), sc, level, sl }{}
		UAException( const UAException& e )ι:Exception{e.what(), e.Code, e.Level(), e.Stack().front()}{}
		Ω Message( UA_StatusCode x )ι->const char*{ return UA_StatusCode_name(x); }

		α Clone()ι->sp<IException> override{ return ms<UAException>(move(*this)); }
		α Move()ι->up<IException> override{ return mu<UAException>(move(*this)); }
		α Ptr()ι->std::exception_ptr override{ return Jde::make_exception_ptr(move(*this)); }
		[[noreturn]] α Throw()->void override{ throw move(*this); }

		α ClientMessage()Ι->string{ return format( "({:x}){}", Code, Message(Code) ); }
	};
}