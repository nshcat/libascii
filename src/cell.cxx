#include <screen.hxx>

void cell::set_fg(const integral_color_type& p_clr)
{
	m_Front = p_clr;
}

void cell::set_fg(const float_color_type& p_clr)
{
	m_Front = integral_color_type(p_clr.r*255.f, p_clr.g*255.f, p_clr.b*255.f);
}

auto cell::fg() const
	-> const integral_color_type&
{
	return m_Front;
}

void cell::set_bg(const integral_color_type& p_clr)
{
	m_Back = p_clr;
}

void cell::set_bg(const float_color_type& p_clr)
{
	m_Back = integral_color_type(p_clr.r*255.f, p_clr.g*255.f, p_clr.b*255.f);
}

auto cell::bg() const
	-> const integral_color_type&
{
	return m_Back;
}

void cell::set_glyph(glyph_type p_glyph)
{
	m_GlyphData &= ~(internal::glyph_mask);
	m_GlyphData |= (p_glyph & internal::glyph_mask);
}

auto cell::glyph() const
	-> glyph_type
{
	return (m_GlyphData & internal::glyph_mask);
}

void cell::set_light_mode(light_mode p_mode)
{
	m_Data &= ~internal::light_mode_mask;
	m_Data |= (ut::enum_cast(p_mode) << internal::light_mode_shift);
}

light_mode cell::get_light_mode() const
{
	auto t_value = ((m_Data & internal::light_mode_mask) >> internal::light_mode_shift);
	return ut::enum_cast<light_mode>(t_value);
}

void cell::set_glyph_set(glyph_set p_set)
{
	m_GlyphData &= ~internal::glyph_set_mask;
	m_GlyphData |= (ut::enum_cast(p_set) << internal::glyph_set_shift);
}

glyph_set cell::get_glyph_set() const
{
	auto t_value = ((m_GlyphData & internal::glyph_set_mask) >> internal::glyph_set_shift);
	return ut::enum_cast<glyph_set>(t_value);
}

void cell::set_depth(depth_type p_depth)
{
	m_Data &= ~internal::depth_mask;
	m_Data |= (p_depth & internal::depth_mask); 
}

auto cell::depth() const
	-> depth_type
{
	return ut::narrow_cast<depth_type>(m_Data & internal::depth_mask);
}

void cell::set_gui_mode(bool p_val)
{
	if(p_val)
		m_Data |= internal::gui_mode_bit;
	else
		m_Data &= ~internal::gui_mode_bit;
}

bool cell::gui_mode() const
{
	return ((m_Data & internal::gui_mode_bit) != 0);
}

void cell::set_shadows(shadow_type p_shadows)
{
	m_Data &= ~(internal::drop_shadow_mask);
	m_Data |= p_shadows;
}

auto cell::shadows() const
	-> shadow_type
{
	return (m_Data & internal::drop_shadow_mask);
}
