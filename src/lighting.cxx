#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <ut/cast.hxx>

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
		if(m_LightCount > 0)
		{
			for(::std::size_t t_index = 0, t_count = 0; t_index < m_Lights.size() && t_count < m_LightCount; ++t_index)
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
		}
		
		// Write light count	 
		glBufferSubData(GL_UNIFORM_BUFFER, state_size + (max_lights * light_size), 4, &m_LightCount);
	
		// Unbind buffer
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		// Clear dirty flag
		m_Dirty = false;
	}
}


auto light_manager::create_light(const light& p_light)
	-> handle_type
{
	if(!has_space())
		throw ::std::runtime_error("Not enough space left to allocate light");
		
	// Search first free handle. We are guarantueed to find something here.
	const auto t_it = ::std::find(m_Used.begin(), m_Used.end(), false);
	
	
	handle_type t_handle = ut::narrow_cast<handle_type>(::std::distance(m_Used.begin(), t_it));
	
	// Insert light data
	m_Lights[t_handle] = p_light;
	
	return t_handle;
}


bool light_manager::has_space(::std::size_t p_amount) const
{
	return (max_lights - m_LightCount) >= p_amount;
}


light& light_manager::modify_light(handle_type p_handle)
{
	// First check if the handle is valid
	if(!check_handle(p_handle) || !m_Used[p_handle])
		throw ::std::runtime_error("Invalid handle");
		
	// Light state is now considered dirty.
	m_Dirty = true;
	
	return m_Lights[p_handle];
}


lighting_state& light_manager::modify_state()
{
	m_Dirty = true;
	
	return m_State;
}


bool light_manager::check_handle(handle_type p_handle) const
{
	return p_handle < max_lights;
}

