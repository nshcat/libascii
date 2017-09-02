#pragma once

#include "screen.hxx"
#include "uniform.hxx"
#include "program.hxx"
#include "texture_set.hxx"
#include "lighting.hxx"
#include "render_context.hxx"
#include "empty_vbo.hxx"

class render_manager
{
	using dimension_type = glm::uvec2;
	using handle_type = render_context::handle_type;
	using texture_type = const texture_set&;
	using context_type = render_context&;

	public:
		render_manager(	render_context& p_context,
						const dimension_type& p_screenDims,
						const texture_type& p_texMgr );
		
	public:		
		auto screen()
			-> screen_manager&;
			
		auto light_manager()
			-> light_manager&;
			
		auto render()
			-> void;
	
	private:
		auto set_uniforms()
			-> void;
					
	private:
		context_type m_Context;
		texture_type m_Tex;
		gl::program m_Program;
		class light_manager m_Lighting;
		screen_manager m_Screen;
		empty_vbo m_Vbo;
		dimension_type m_GlyphCount;
};

