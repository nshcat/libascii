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
		
		// Copy state
		glBufferSubData(GL_UNIFORM_BUFFER, 0, state_size, static_cast<const void*>(&m_State));
	
		// Copy all active lights
		for(::std::size_t t_index = 0, t_count = 0; t_index < m_Lights.size(); ++t_index)
		{
			// Only write light if it is used
			if(!m_Used[t_index])
				continue;
				
			// Write light data to buffer
			glBufferSubData(GL_UNIFORM_BUFFER,
							state_size + (t_count * light_size),
							light_size,
							static_cast<const void*>(&m_Lights[t_index])
			);
			
			++t_count;
		}
		
		// Write light count	 
		glBufferSubData(GL_UNIFORM_BUFFER, state_size + (max_lights * light_size), 4, &m_LightCount);
	
		// Unbind buffer
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		// Clear dirty flag
		m_Dirty = false;
	}
}
