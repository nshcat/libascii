// TODO: put_string_fmt (with color formatting! %C)

#pragma once
#include <cctype>
#include <string_view>
#include <screen.hxx>


template< typename... Ts >
auto sequence(Ts&&... p_funcs)
{
	return [p_funcs...](cell& p_cell) -> void
	{
		const auto t_x = { (p_funcs(p_cell), 0)... };
	};
}

static auto clear()
{
	return [](cell& p_cell) -> void
	{
		p_cell = cell{ };
	};
}

static auto put_string(::std::string_view p_str, cell::integral_color_type p_front, cell::integral_color_type p_back = { })
{
	return [p_str, p_front, p_back](cell& p_cell) mutable
	{
		if(p_str.length() > 0)
		{
			// Only printable ascii chars are allowed.
			if(! ::std::isprint(p_str.front()))
				throw ::std::runtime_error("put_string: Encountered non-printable character!");
			
			// Since the character is printable, just converting it to a glyph is valid
			p_cell.set_glyph(p_str.front());
			p_cell.set_fg(p_front);
			p_cell.set_bg(p_back);
			
			// Remove char from string	
			p_str.remove_prefix(1);
		}
	};
}

static auto draw(cell::glyph_type p_glyph, cell::integral_color_type p_front, cell::integral_color_type p_back = { })
{
	return [p_glyph, p_front, p_back](cell& p_cell)
	{
		p_cell.set_fg(p_front);
		p_cell.set_bg(p_back);
		p_cell.set_glyph(p_glyph);
	};
}

static auto set_light_mode(light_mode p_mode)
{
	return [p_mode](cell& p_cell)
	{
		p_cell.set_light_mode(p_mode);
	};
}

static auto set_gui_mode(bool p_flag)
{
	return [p_flag](cell& p_cell)
	{
		p_cell.set_gui_mode(p_flag);
	};
}
