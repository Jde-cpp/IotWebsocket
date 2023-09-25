

namespace Jde::Iot
{
	struct UAException : Exception
	{
		UAException( UA_StatusCode x, ELogLevel level=ELogLevel::Debug, SRCE ):Exception{ Message(x), x, level, sl }{}
		Ω Message( UA_StatusCode x )ι->const char*{ return UA_StatusCode_name(x); }
	};
}