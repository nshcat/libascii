#pragma once

#include <string>
#include <glm/glm.hpp>

// A class managing the ascii glyph sheet
class texture
{
	using size_type = ::std::size_t;

	public:
		// Dimensions of glyph sheet, in glyphs
		const static size_type sheet_width = 16;
		const static size_type sheet_height = 16;
		
	public:
		texture(const ::std::string& p_path);
		~texture();

	public:
		// Returns dimensions of a single glyph texture
		auto glyph_size() const
			-> glm::ivec2;
			
		auto use() const
			-> void;

	private:
		GLuint m_Tex;
		glm::ivec2 m_GlyphDim;
		
};
