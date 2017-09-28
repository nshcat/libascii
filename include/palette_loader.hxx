#pragma once

#include <string>

#include "asset_manager.hxx"
#include "palette.hxx"

template<>
struct asset_loader <palette>
{
	auto load_asset(const ::std::string& p_name) const
		-> palette;
};
