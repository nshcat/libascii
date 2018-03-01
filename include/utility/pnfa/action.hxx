#pragma once

#include <type_traits>
#include <functional>
#include <ut/type_traits.hxx>

#include "utility.hxx"

namespace utility::pnfa
{
	namespace internal
	{
		class nothing_t
		{			
			public:
				template< typename... Ts >
				auto operator()(Ts&&...)
					-> void
				{
					return;
				}
		};
	}

	auto nothing()
		-> internal::nothing_t
	{
		return { };
	}
}
