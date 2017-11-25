#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <ut/format.hxx>
#include <diagnostics.hxx>
#include <renderer.hxx>
#include <global_state.hxx>

/*render_manager::render_manager(	render_context& p_context,
								const dimension_type& p_screenDims,
								const texture_set& p_texMgr )
	: 	. m_Tex{p_texMgr},
		. m_Screen{p_screenDims},
		. m_Lighting{ },
		. m_Vbo{ },
		. m_Context{p_context},
		. m_GlyphCount{p_screenDims},
		m_Program{
			gl::vertex_shader{ gl::from_file, "assets/shaders/ascii.vs.glsl" },
			gl::fragment_shader{ gl::from_file, "assets/shaders/ascii.fs.glsl" },
		}	
{
	p_context.resize({p_texMgr.glyph_size().x * p_screenDims.x, p_texMgr.glyph_size().y * p_screenDims.y});
	m_Program.use();
	set_uniforms();
}*/

auto render_manager::initialize()
	-> void
{
	post_diagnostic(message_type::info, "render_manager", "initialization started");

	// TODO texture_set_loader
	m_Tex = texture_set{
		shadow_texture("assets/textures/default/shadows.png"),
		text_texture("assets/textures/default/text.png"),
		graphics_texture("assets/textures/default/graphics.png")
	};
	
	post_diagnostic(message_type::info, "render_manager", ut::sprintf("texture glyph size: (%u, %u)", m_Tex.glyph_size().x, m_Tex.glyph_size().y));
	
	// TODO program_loader
	m_Program = gl::program{
		gl::vertex_shader{ gl::from_file, "assets/shaders/ascii.vs.glsl" },
		gl::fragment_shader{ gl::from_file, "assets/shaders/ascii.fs.glsl" }
	};
	
	m_Screen.initialize();
	m_GlyphCount = m_Screen.screen_size();

	global_state<render_context>().resize({m_Tex.glyph_size().x * m_GlyphCount.x, m_Tex.glyph_size().y * m_GlyphCount.y});

	m_Vbo.initialize();

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
	auto& t_context = global_state<render_context>();

	// Set uniforms		
	gl::set_uniform(m_Program, "fog_color", glm::vec4{ 0.1f, 0.1f, 0.3f, 1.f });
	gl::set_uniform(m_Program, "fog_density", .15f);
	gl::set_uniform(m_Program, "projection_mat", glm::ortho(0.f, static_cast<float>(t_context.dimensions().x), static_cast<float>(t_context.dimensions().y), 0.f, -1.f, 1.f));		
	gl::set_uniform(m_Program, "sheet_dimensions", glm::ivec2{ texture_set::sheet_width, texture_set::sheet_height });
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
	global_state<light_manager>().sync();
	m_Screen.sync();
	
	// Reset openGl state
	m_Program.use();
	m_Vbo.use();
	m_Tex.use();
	
	// Render
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_GlyphCount.x * m_GlyphCount.y);
}
