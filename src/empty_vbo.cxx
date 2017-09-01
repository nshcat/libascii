#include <empty_vbo.hxx>

empty_vbo::empty_vbo()
{
	glGenBuffers(1, &m_VBOHandle);
	glGenVertexArrays(1, &m_VAOHandle);
	use();
}

auto empty_vbo::use() const
	-> void
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOHandle);
	glBindVertexArray(m_VAOHandle);
}
