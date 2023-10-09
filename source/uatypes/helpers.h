#pragma once

namespace Jde::Iot
{
	using NamespaceId = uint16;

	Ξ ToSV( const UA_String& s )ι->sv{ return sv{ (const char*)s.data, s.length }; }
	Ξ ToUV( sv s )ι->UA_String{ return { s.size(), (UA_Byte*)s.data() }; }
	constexpr α operator "" _uv( const char* x, unsigned long len )ι->UA_String{ return UA_String{ len, (UA_Byte*)x }; }
}