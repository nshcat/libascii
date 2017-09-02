#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <renderer.hxx>

render_manager::render_manager(	render_context& p_context,
								const dimension_type& p_screenDims,
								const texture_manager& p_texMgr )
	: 	m_Tex{p_texMgr},
		m_Screen{p_screenDims},
		m_Lighting{ },
		m_Vbo{ },
		m_Context{p_context},
		m_GlyphCount{p_screenDims},
		m_Program{
			gl::vertex_shader{ gl::from_file, "assets/shaders/ascii.vs.glsl" },
			gl::fragment_shader{ gl::from_file, "assets/shaders/ascii.fs.glsl" },
		}	
{
	p_context.resize({p_texMgr.glyph_size().x * p_screenDims.x, p_texMgr.glyph_size().y * p_screenDims.y});
	m_Program.use();
	set_uniforms();
}


auto render_manager::screen()
	-> screen_manager&
{
	return m_Screen;
}

auto render_manager::set_uniforms()
	-> void
{
	// Set uniforms		
	gl::set_uniform(m_Program, "fog_color", glm::vec4{ 0.1f, 0.1f, 0.3f, 1.f });
	gl::set_uniform(m_Program, "fog_density", .15f);
	gl::set_uniform(m_Program, "projection_mat", glm::ortho(0.f, static_cast<float>(m_Context.dimensions().x), static_cast<float>(m_Context.dimensions().y), 0.f, -1.f, 1.f));		
	gl::set_uniform(m_Program, "sheet_dimensions", glm::ivec2{ texture_manager::sheet_width, texture_manager::sheet_height });
	gl::set_uniform(m_Program, "glyph_dimensions", m_Tex.glyph_size());
	gl::set_uniform(m_Program, "glyph_count", glm::ivec2{m_GlyphCount});
	
	// Samplers are just integers
	gl::set_uniform(m_Program, "text_texture", 0);
	gl::set_uniform(m_Program, "graphics_texture", 1);	
	gl::set_uniform(m_Program, "shadow_texture", 2);
	gl::set_uniform(m_Program, "input_buffer", 3);
}

auto render_manager::render()
	-> void
{
	// Sync state with gpu
	m_Lighting.sync();
	m_Screen.sync();
	
	// Reset openGl state
	m_Program.use();
	m_Vbo.use();
	m_Tex.use();
	
	// Render
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_GlyphCount.x * m_GlyphCount.y);
}

auto render_manager::light_manager()
	-> class light_manager&
{
	return m_Lighting;
}
