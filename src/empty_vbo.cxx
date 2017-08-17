#include <empty_vbo.hxx>

empty_vbo::empty_vbo()
{
	glGenBuffers(1, &m_Handle);
	use();
}

auto empty_vbo::use() const
	-> void
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
}
