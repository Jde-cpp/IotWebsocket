﻿#pragma once
#include <jde/TypeDefs.h>

namespace Jde::Iot{
	using boost::concurrent_flat_map;
	using boost::concurrent_flat_set;

	using NamespaceId = uint16;
	using OpcId=string;
	using MonitorId=UA_UInt32;
	using SubscriptionId=UA_UInt32;
	using StatusCode = UA_StatusCode;
	template <auto F> //https://stackoverflow.com/questions/19053351/how-do-i-use-a-custom-deleter-with-a-stdunique-ptr-member
	struct DeleterFromFunction {
    Τ constexpr α operator()(T* arg)Ι->void{ F(arg); }
	};

	template <typename T, auto F> using UAUP = std::unique_ptr<T, DeleterFromFunction<F>>;
}