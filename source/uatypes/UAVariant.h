#pragma once

namespace Jde::Iot
{
	struct UAVariant
	{
		UA_Variant _value{};
		operator UA_Variant&(){return _value;}
		auto operator &()->UA_Variant*{return &_value;}
		auto data()->void*{ return _value.data; }
	};
}