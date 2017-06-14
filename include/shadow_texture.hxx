#pragma once

#include <string>
#include <glm/glm.hpp>

// A class managing the ascii glyph sheet
class shadow_texture
{
	using size_type = ::std::size_t;
		
	public:
		shadow_texture(const ::std::string& p_path);
		~shadow_texture();

	public:
			
		auto use() const
			-> void;

	private:
		GLuint m_Tex;		
};
