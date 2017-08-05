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
	m_Glyph = p_glyph;
}

auto cell::glyph() const
	-> glyph_type
{
	return m_Glyph;
}
