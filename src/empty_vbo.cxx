#include <empty_vbo.hxx>

auto empty_vbo::initialize()
	-> void
{
	// TODO error handling
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
