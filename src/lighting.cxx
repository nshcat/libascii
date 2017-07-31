#include <lighting.hxx>


light_manager::light_manager()
{
	// Create Buffer on GPU
	glGenBuffers(1, &m_GPUBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m_GPUBuffer);
	glBufferData(GL_UNIFORM_BUFFER, buffer_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); // Unbind buffer
	
	// Bind it to uniform block
	// The light buffer has a fixed binding of 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GPUBuffer);
}

void light_manager::sync()
{
	if(m_Dirty)
	{
		// Bind buffer
		glBindBuffer(GL_UNIFORM_BUFFER, m_GPUBuffer);
	
		// Use glBufferSubData to modify buffer contents
	
	
		// Unbind buffer
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		// Clear dirty flag
		m_Dirty = false;
	}
}
