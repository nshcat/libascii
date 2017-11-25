#pragma once

#include "screen.hxx"
#include "uniform.hxx"
#include "program.hxx"
#include "texture_set.hxx"
#include "render_context.hxx"
#include "empty_vbo.hxx"
#include "global_system.hxx"

class render_manager
	: public global_system
{
	using dimension_type = glm::uvec2;
	using handle_type = render_context::handle_type;

	public:
		render_manager() = default;
		~render_manager() = default;
											
	public:
		auto initialize()
			-> void;
		
	public:		
		auto screen()
			-> screen_manager&;
			
		auto render()
			-> void;
	
	private:
		auto set_uniforms()
			-> void;
					
	private:
		texture_set m_Tex;
		gl::program m_Program{gl::defer_creation};
		screen_manager m_Screen;
		empty_vbo m_Vbo;
		dimension_type m_GlyphCount;
};

