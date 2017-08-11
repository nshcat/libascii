#pragma once

#include <stdexcept>
#include <GLXW/glxw.h>
#include "shader.hxx"


namespace gl
{
	// Shader program abstraction
	class program final
	{
		public:
			template< internal::shader_type... Types >
			program(const internal::shader<Types>&... p_shaders)
				: program()
			{
				auto x = { (this->attach(p_shaders),0)... }; 
				(void)x;
				
				link();
			}
			
			program()
				: m_Handle{glCreateProgram()}
			{
				if(!m_Handle)
					throw ::std::runtime_error("Failed to create program!");
			}
			
		public:
			~program()
			{
				if(m_Handle)
					glDeleteProgram(m_Handle);
			}
		
			program(const program&) = delete;
			program& operator=(const program&) = delete;
			
			program(program&&) = default;
			program& operator=(program&&) = default;
		
		public:
			template< internal::shader_type Type >
			auto attach(const internal::shader<Type>& p_shader)
				-> void
			{
				glAttachShader(m_Handle, p_shader.handle());
			}
			
			auto link()
				-> void
			{
				glLinkProgram(m_Handle);
				
				if(query(GL_LINK_STATUS) == GL_FALSE)
					throw shader_exception("Failed to link shader program!", info_log());
			}
			
			auto handle() const
				-> GLuint
			{
				return m_Handle;
			}
			
			auto use() const
				-> void
			{
				glUseProgram(m_Handle);
			}
			
			auto query(GLenum p_name) const
				-> GLint
			{
				GLint t_result;
				glGetProgramiv(handle(), p_name, &t_result);
				return t_result;
			}
			
			auto info_log() const
				-> ::std::string
			{
				// Query info log len
				auto t_len = query(GL_INFO_LOG_LENGTH);
					
				if(t_len == 0) return {};
					
				// Create buffer
				::std::vector<char> t_buf(t_len+1);
					
				// Fetch info log
				GLsizei t_result_length{};
				glGetProgramInfoLog(	m_Handle,
										t_len,
										&t_result_length,
										t_buf.data()
									);
										
				return{ t_buf.begin(), t_buf.end() };		
			}
	
		private:
			GLuint m_Handle;
	};
}
