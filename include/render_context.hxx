// Provides glfw and glxw initialization

#pragma once

#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class render_context
{
	public:
		using handle_type = GLFWwindow*;
		using dimension_type = glm::uvec2;

	public:
		render_context();
		~render_context();
		
	public:
		auto handle() const
			-> handle_type;
			
		auto dimensions() const
			-> const dimension_type&;
			
		auto resize(const dimension_type&)
			-> void;
			
		auto should_close() const
			-> bool;
			
		auto pump_events()
			-> void;
			
	private:
		auto init_glfw()
			-> void;
			
		auto init_glxw()
			-> void;
			
		auto init_debug()
			-> void;
			
		auto report_version()
			-> void;
			
	private:
		handle_type m_WindowHandle{};
		dimension_type m_WindowSize{100, 100};
};

