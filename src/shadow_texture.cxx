#include <stdexcept>

#include <GLXW/glxw.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <shadow_texture.hxx>


SDL_Surface* load_texture(const ::std::string& p_path);

GLuint make_gl_texture(SDL_Surface* p_srfc);

shadow_texture::shadow_texture(const ::std::string& p_str)
{
	glActiveTexture(GL_TEXTURE2);
	const auto t_Srfc = load_texture(p_str);
	m_Tex = make_gl_texture(t_Srfc);
}

shadow_texture::~shadow_texture()
{
	glDeleteTextures(1, &m_Tex);
}

void shadow_texture::use() const
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_Tex);
}


