#pragma once

namespace Jde::Iot
{
	struct OpcServer
	{
		OpcServer( string&& address )ι:Address{address}{}
		OpcServer( const DB::IRow& r )ι;
		Ω Select()ι->AsyncAwait;
		Ω Select( string id )ι->AsyncAwait;

		uint32 Id;
		string Address;
		bool IsDefault;
		string Name;
		string Target;
	};
}