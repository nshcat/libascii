// TODO: put_string_fmt (with color formatting! %C)
// TODO: draw(...) with libcl-like tags for foreground, background, glyph and glyph_set!
//		 => that is good because no order has to be memorized and we can easily SUPPORT DEFAULT VALUES

#pragma once
#include <cctype>
#include <algorithm>
#include <string_view>
#include <screen.hxx>
#include <utility.hxx>


// TODO: fog, shadows
namespace internal
{
	struct draw_parameters
	{
		cell::integral_color_type m_Foreground{0U};
		cell::integral_color_type m_Background{0U};
		cell::glyph_type m_Glyph{0U};
		glyph_set m_GlyphSet{glyph_set::text};
	};
}

using foreground = internal::tag_base<
					internal::draw_parameters,
					cell::integral_color_type,
					&internal::draw_parameters::m_Foreground,
					struct foreground_tag_t
				   >;
				   
using background = internal::tag_base<
					internal::draw_parameters,
					cell::integral_color_type,
					&internal::draw_parameters::m_Background,
					struct background_tag_t
				   >;

using glyph =	   internal::tag_base<
					internal::draw_parameters,
					cell::glyph_type,
					&internal::draw_parameters::m_Glyph,
					struct glyph_tag_t
				   >;
				   
using set =  	   internal::tag_base<
					internal::draw_parameters,
					glyph_set,
					&internal::draw_parameters::m_GlyphSet,
					struct glyph_set_tag_t
				   >; 
				 


template< 	typename... Ts,
			typename = ::std::enable_if_t<
				::std::conjunction_v<
					::std::is_same<
						typename Ts::target_type,
						internal::draw_parameters
					>...	
				>
			>
>
auto draw(Ts&&... p_tags)
{
	internal::draw_parameters t_params{ };
	
	auto x = { (p_tags.apply(t_params), 0)... };
	
	return [t_params](cell& p_cell) -> void
	{
		p_cell.set_fg(t_params.m_Foreground);
		p_cell.set_bg(t_params.m_Background);
		p_cell.set_glyph(t_params.m_Glyph);
		p_cell.set_glyph_set(t_params.m_GlyphSet);
	};
}

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

static auto set_glyph_set(glyph_set p_set)
{
	return [p_set](cell& p_cell)
	{
		p_cell.set_glyph_set(p_set);
	};
}

template<	typename... Ts,
			typename = ::std::enable_if_t<
				::std::conjunction_v<
					::std::is_same<Ts, drop_shadow>...
				>
			>
>
auto set_shadows(Ts... p_args)
{
	::std::initializer_list<::std::uint32_t> t_lst = { ut::enum_cast(p_args)... };
	
	const auto t_value = ::std::accumulate(
		t_lst.begin(), t_lst.end(), ::std::uint32_t{}, [](const auto& p_a, const auto& p_b) { return p_a | p_b; }
	);

	return [t_value](cell& p_cell)
	{
		p_cell.set_shadows(t_value);
	};
}

template<	typename... Ts,
			typename = ::std::enable_if_t<
				::std::conjunction_v<
					::std::is_same<Ts, drop_shadow>...
				>
			>
>
auto add_shadows(Ts... p_args)
{
	::std::initializer_list<::std::uint32_t> t_lst = { ut::enum_cast(p_args)... };
	
	const auto t_value = ::std::accumulate(
		t_lst.begin(), t_lst.end(), ::std::uint32_t{}, [](const auto& p_a, const auto& p_b) { return p_a | p_b; }
	);

	return [t_value](cell& p_cell)
	{
		p_cell.set_shadows(p_cell.shadows() | t_value);
	};
}

static auto highlight()
{
	return [](cell& p_cell)
	{
		::std::swap(p_cell.m_Front, p_cell.m_Back);
	};
}
