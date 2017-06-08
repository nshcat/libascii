#include <stdexcept>

#include <GLXW/glxw.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <texture.hxx>

// TODO SDL error handling from sdl_cpu

SDL_Surface* canonicalize_format(SDL_Surface *src, bool convert_magenta)
{
	SDL_PixelFormat fmt;
	fmt.palette = NULL;
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
	fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	fmt.Rshift = 24; fmt.Gshift = 16; fmt.Bshift = 8; fmt.Ashift = 0;
	#else
	fmt.Rshift = 0; fmt.Gshift = 8; fmt.Bshift = 16; fmt.Ashift = 24;
	#endif
	fmt.Rmask = 255 << fmt.Rshift;
	fmt.Gmask = 255 << fmt.Gshift;
	fmt.Bmask = 255 << fmt.Bshift;
	fmt.Amask = 255 << fmt.Ashift;
	//fmt.colorkey = 0;
	//fmt.alpha = 255;

	if (src->format->Amask == 0 && convert_magenta)
	{ // No alpha
		SDL_SetColorKey(src, SDL_TRUE, SDL_MapRGB(src->format, 255, 0, 255));
	}
	
	SDL_Surface *tgt = SDL_ConvertSurface(src, &fmt, SDL_SWSURFACE);
	SDL_FreeSurface(src);
	return tgt;
}

SDL_Surface* load_texture(const ::std::string& p_path)
{
	SDL_Surface* image = IMG_Load(p_path.c_str());
	
	if(!image)
		throw ::std::runtime_error("Texture not found!");
	
	// Determine glyph dimensions
	//m_GlyphDim = glm::uvec2{ image->w / texture::sheet_width, image->h / texture::sheet_height };
	
	SDL_Surface* tex = canonicalize_format(image, true);
	
	return tex;
}

GLuint make_gl_texture(SDL_Surface* p_srfc)
{
	GLuint TextureID = 0;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p_srfc->w, p_srfc->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, p_srfc->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	SDL_FreeSurface(p_srfc);
	
	return TextureID;
}

texture::texture(const ::std::string& p_str)
{
	const auto t_Srfc = load_texture(p_str);
	m_Tex = make_gl_texture(t_Srfc);
	
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_GlyphDim.x);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_GlyphDim.y);
	
	m_GlyphDim /= glm::ivec2{ texture::sheet_width, texture::sheet_height };
}

texture::~texture()
{
	glDeleteTextures(1, &m_Tex);
}

void texture::use() const
{
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, m_Tex);
}

glm::ivec2 texture::glyph_size() const
{
	return m_GlyphDim;
}
