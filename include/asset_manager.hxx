#pragma once

#include <ut/always_false.hxx>

#include "global_system.hxx"

template< typename T >
struct asset_loader
{
	static_assert(ut::always_false_v<T>,
		"No asset loader registered for given asset type T");
};


class asset_manager
	: public global_system
{
	public:
		auto initialize()
			-> void;
			
	public:
		template< typename T, typename... Ts >
		auto load_asset(Ts&&... p_args)
			-> T
		{
			// Create asset loader instance
			asset_loader<T> t_loader{ };
			
			// Delegate loading of asset to loader instance
			return t_loader.load_asset(::std::forward<Ts>(p_args)...);
		}
};
