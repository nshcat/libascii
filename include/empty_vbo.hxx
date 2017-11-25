#pragma once

#include <GLXW/glxw.h>

class empty_vbo
{
	using handle_type = GLuint;

	public:
		empty_vbo() = default;
		
	public:
		auto initialize()
			-> void;
		
	public:
		auto use() const
			-> void;
			
	private:
		handle_type m_VBOHandle;
		handle_type m_VAOHandle;
};
