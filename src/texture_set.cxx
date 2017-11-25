#include <stdexcept>

#include <GLXW/glxw.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <texture_set.hxx>

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

GLuint make_gl_texture(const ::std::string& p_path)
{
	GLuint TextureID = 0;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	
	auto* t_srfc = load_texture(p_path);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t_srfc->w, t_srfc->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t_srfc->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	SDL_FreeSurface(t_srfc);
	
	return TextureID;
}

texture_set::texture_set(texture_set&& p_set)
{
	::std::swap(m_TextTex, p_set.m_TextTex);
	::std::swap(m_GfxTex, p_set.m_GfxTex);
	::std::swap(m_ShadowTex, p_set.m_ShadowTex);
	::std::swap(m_GlyphDim, p_set.m_GlyphDim);
}

texture_set& texture_set::operator=(texture_set&& p_set)
{
	::std::swap(m_TextTex, p_set.m_TextTex);
	::std::swap(m_GfxTex, p_set.m_GfxTex);
	::std::swap(m_ShadowTex, p_set.m_ShadowTex);
	::std::swap(m_GlyphDim, p_set.m_GlyphDim);
	
	return *this;
}

texture_set::~texture_set()
{
	if(m_TextTex)
		glDeleteTextures(1, &m_TextTex);
		
	if(m_ShadowTex)
		glDeleteTextures(1, &m_ShadowTex);
	
	if(m_GfxTex)
		glDeleteTextures(1, &m_GfxTex);
}

void texture_set::dispatch(const internal::shadow_texture_t& p_tag)
{
	m_ShadowTex = make_gl_texture(p_tag.m_Path);
	// TODO: size check
}

void texture_set::dispatch(const internal::text_texture_t& p_tag)
{
	m_TextTex = make_gl_texture(p_tag.m_Path);
	
	update_dims(recv_dims(m_TextTex));
}

void texture_set::dispatch(const internal::graphics_texture_t& p_tag)
{
	m_GfxTex = make_gl_texture(p_tag.m_Path);
	
	update_dims(recv_dims(m_GfxTex));
}

void texture_set::update_dims(const dimension_type& p_dims)
{
	if(m_GlyphDim != dimension_type{})
	{
		if(m_GlyphDim != p_dims)
		{
			ut::throwf<::std::runtime_error>("Glyph sheet tile size mismatch: %ux%u != %ux%u",
				p_dims.x, p_dims.y, m_GlyphDim.x, m_GlyphDim.y);
		}
	}
	else m_GlyphDim = p_dims;
}


void texture_set::use() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextTex);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (m_GfxTex != 0 ? m_GfxTex : m_TextTex));
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_ShadowTex);
}

auto texture_set::glyph_size() const
	-> const dimension_type&
{
	return m_GlyphDim;
}

auto texture_set::recv_dims(GLuint p_tex)
	-> dimension_type
{
	dimension_type t_dims{};

	glGetTextureLevelParameteriv(p_tex, 0, GL_TEXTURE_WIDTH, &t_dims.x);
	glGetTextureLevelParameteriv(p_tex, 0, GL_TEXTURE_HEIGHT, &t_dims.y);	
	
	return t_dims / dimension_type{ texture_set::sheet_width, texture_set::sheet_height };
}

internal::shadow_texture_t shadow_texture(const ::std::string& p_path)
{
	return { p_path };
}

internal::text_texture_t text_texture(const ::std::string& p_path)
{
	return { p_path };
}

internal::graphics_texture_t graphics_texture(const ::std::string& p_path)
{
	return { p_path };
}

