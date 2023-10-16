#pragma once
#include <boost/algorithm/hex.hpp>

#define var const auto
namespace Jde::Iot{
	using NamespaceId = uint16;

	Ξ ToSV( const UA_String& s )ι->sv{ return sv{ (const char*)s.data, s.length }; }
	Ξ ToUV( sv s )ι->UA_String{ return { s.size(), (UA_Byte*)s.data() }; }
	//Ξ mum( sv s )ι->UA_String{ return { s.size(), (UA_Byte*)s.data() }; }
	Ŧ Zero( T& x )ι->void{ ::memset( &x, 0, sizeof(T) ); }
	constexpr α operator "" _uv( const char* x, unsigned long len )ι->UA_String{ return UA_String{ len, (UA_Byte*)x }; }
	Ξ ToJson( UA_UInt64 v )->json{ return json{ {"high", v>>32}, {"low", v&0xFFFFFFFF}, {"unsigned",true} }; };
	Ξ ToJson( UA_Int64 v )->json{ return json{ {"high", v>>32}, {"low", v&0xFFFFFFFF}, {"unsigned",false} }; };
	Ξ ToJson( UA_Guid v )->json{ boost::uuids::uuid id; memcpy(&id.data, &v, id.size() ); return json{ boost::uuids::to_string(id) }; }
	Ξ ByteStringToJson( const UA_ByteString& v )->json{ string hex; hex.reserve( v.length*2 ); boost::algorithm::hex_lower( ToSV(v), std::back_inserter(hex) ); return json{hex}; }

	struct UADateTime
	{
		UADateTime( const UA_DateTime& dt )ι:_dateTime{dt}{}
		α ToJson()ι->json{
			var dts = UA_DateTime_toStruct( _dateTime );
			var seconds = Clock::to_time_t( Chrono::ToTimePoint(dts.year, dts.month, dts.day, dts.hour, dts.min, dts.sec) );
			var nanos = dts.milliSec*TimeSpan::MicrosPerMilli*TimeSpan::NanosPerMicro+dts.microSec*TimeSpan::NanosPerMicro+dts.nanoSec;
			return json{ {"seconds", seconds}, {"nanos", nanos} };
		}
	private:
			UA_DateTime _dateTime;
	};
}
#undef var