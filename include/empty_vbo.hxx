#pragma once

#include <GLXW/glxw.h>

class empty_vbo
{
	using handle_type = GLuint;

	public:
		empty_vbo();
		
	public:
		auto use() const
			-> void;
			
	private:
		handle_type m_VBOHandle;
		handle_type m_VAOHandle;
};
