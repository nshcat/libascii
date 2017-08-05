#pragma once

#include <string_view>
#include <screen.hxx>


template< typename... Ts >
auto sequence(Ts&&... p_funcs)
{
	return [p_funcs...](cell& p_cell) -> void
	{
		const auto t_x = { (p_func(p_cell), 0)... };
	};
}

static auto clear()
{
	return [](cell& p_cell) -> void
	{
		p_cell = cell{ };
	};
}

static auto put_string(string_view p_str)
{
	return [p_str](cell& p_cell) mutable
	{
		if(p_str.length() > 0)
		{
			// print char	
			
			// Remove char from string	
			p_str.remove_prefix(1);
		}
	};
}
