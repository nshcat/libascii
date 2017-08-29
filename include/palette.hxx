#pragma once

#include <array>
#include <string>
#include <cstdint>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ut/cast.hxx>
#include <ut/array_view.hxx>

enum class color
	: ::std::uint8_t
{
	black = 0,
	light_gray,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	
	dark_gray,
	white,
	light_red,
	light_green,
	light_yellow,
	light_blue,
	light_magenta,
	light_cyan
};

class palette
{
	using color_type = glm::uvec3;
	
	public:
		static constexpr ::std::size_t num_colors = 16U;

	public:
		palette(const ::std::string&);
		palette(ut::array_view<color_type>);
		
	public:
		auto lookup(color) const
			-> const color_type&;

	private:
		::std::array<color_type, num_colors> m_ColorData;
};
