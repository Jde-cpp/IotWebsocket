#pragma once

#define UAε(x) var sc = x; if( sc ) throw UAException{ sc, ELogLevel::Error };

namespace Jde::Iot
{
	struct UAException : Exception
	{
		UAException( UA_StatusCode x, ELogLevel level=ELogLevel::Debug, SRCE )ι:Exception{ Message(x), x, level, sl }{}
		UAException( const UAException& e):Exception{e.what(), e.Code, e.Level(), e.Stack().front()}{}
		Ω Message( UA_StatusCode x )ι->const char*{ return UA_StatusCode_name(x); }
	};
}