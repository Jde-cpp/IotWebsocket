#pragma once

namespace Jde::Iot
{
	struct OpcServer
	{
		OpcServer( str address )ι:Url{address}{}
		OpcServer( const DB::IRow& r )ι;
		Ω Select()ι->AsyncAwait;
		Ω Select( string id )ι->AsyncAwait;

		uint32 Id;
		string Url;
		bool IsDefault;
		string Name;
		string Target;
	};
}