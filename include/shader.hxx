// TODO merge program and shader to allow post-mortem warning checks etc

#pragma once

#include <stdexcept>
#include <limits>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <GLXW/glxw.h>

#include <ut/string_view.hxx>


namespace ut
{
	namespace gl
	{
		class shader_exception
			: public ::std::runtime_error
		{
			using base_type = ::std::runtime_error;
			
			public:
				explicit shader_exception(const ::std::string& p_arg, const ::std::string& p_log)
					: base_type(p_arg), m_Log{p_log}
				{
				}
				
				explicit shader_exception(const char* p_arg, const ::std::string& p_log)
					: base_type(p_arg), m_Log{p_log}
				{
				}
			
				explicit shader_exception(const ::std::string& p_arg)
					: base_type(p_arg)
				{
				}
				
				explicit shader_exception(const char* p_arg)
					: base_type(p_arg)
				{
				}
				
			public:
				auto log() const
					-> const ::std::string&
				{
					return m_Log;
				}
				
			private:
				::std::string m_Log;
		};
	
		namespace internal
		{
			enum class shader_type
				: GLenum
			{
				vertex = GL_VERTEX_SHADER,
				fragment = GL_FRAGMENT_SHADER,
				tess_control = GL_TESS_CONTROL_SHADER,
				tess_evaluation = GL_TESS_EVALUATION_SHADER,
				geometry = GL_GEOMETRY_SHADER,
				compute = GL_COMPUTE_SHADER			
			};
			
			
			// Tag dispatch helpers
			struct from_text_t
			{
			};			
			
			struct from_file_t
			{
			};
			// ----
			
			
			// RAII Shader wrapper
			template< shader_type Type >
			class shader final
			{
				using Tthis = shader<Type>;
			
				public:
					// Create new shader without any source code.
					shader()
						: m_Handle{ glCreateShader(
									static_cast<GLenum>(Type) )}
					{
						if(!m_Handle)
							throw shader_exception("Failed to create shader!");
						else if(m_Handle == GL_INVALID_ENUM)
							throw shader_exception("Failed to create shader: Invalid shader type!");
					}

					shader(from_text_t, string_view p_text)
						: shader()
					{
						source(from_text_t{}, p_text);
						compile();		
					}
					
					shader(from_file_t, string_view p_path)
						: shader()
					{
						source(from_file_t{}, p_path);
						compile();
						
					}
				
					// Destroy shader
					~shader()
					{
						if(m_Handle != 0u)
							glDeleteShader(m_Handle);
					}
				
					// Disallow copy since this manages lifetime.
					shader(const Tthis&) = delete;
					Tthis& operator=(const Tthis&) = delete;
					
					// Default move.
					shader(Tthis&&) = default;
					Tthis& operator=(Tthis&&) = default;
					
				public:
					auto source(from_file_t, string_view p_path)
						-> void
					{
						// Open file in text mode
						::std::ifstream t_file{ p_path.to_string() };
						
						if(t_file.fail())
							throw shader_exception("Failed to open shader file");
						
						// Determine size
						t_file.ignore(::std::numeric_limits<::std::streamsize>::max());				
						auto t_length = t_file.gcount();
						
						// Prepare file for read. EOF was set and has to be cleared.
						t_file.clear();
						t_file.seekg(0, ::std::ios_base::beg);
						
						// Create buffer.
						::std::vector<char> t_buf;
						t_buf.reserve(t_length);
						
						// Read file contents
						::std::copy(	::std::istream_iterator<char>(t_file>>::std::noskipws),
										::std::istream_iterator<char>(),
										::std::back_inserter(t_buf)
									);
									
						// Hand over data
						return source(from_text_t{}, { t_buf.data(), t_buf.size() });				
					}
					
					auto source(from_text_t, string_view p_text)
						-> void
					{
						auto t_data = p_text.data();
						GLint t_len = p_text.length();
					
						glShaderSource(		m_Handle,
											1,
											&t_data,
											&t_len
										);																		
					}
		
				public:
					auto query(GLenum p_name)
						-> GLint
					{
						GLint t_result;					
						glGetShaderiv(m_Handle, p_name, &t_result);					
						return t_result;
					}
					
				private:
					auto info_log()
						-> ::std::string
					{
						// Query info log len
						auto t_len = query(GL_INFO_LOG_LENGTH);
						
						if(t_len == 0) return {};
						
						// Create buffer
						::std::vector<char> t_buf(t_len+1);
						
						// Fetch info log
						GLsizei t_result_length{};
						glGetShaderInfoLog(		m_Handle,
												t_len,
												&t_result_length,
												t_buf.data()
											);
											
						return{ t_buf.begin(), t_buf.end() };											
					}
					
				public:
					auto compile()
						-> void
					{
						glCompileShader(m_Handle);
						
						const auto t_log = info_log();
						
						if(query(GL_COMPILE_STATUS) == GL_FALSE)
							throw shader_exception("Failed to compile shader!", t_log);
						
						// If log is not empty, there were some warnings.
						if(!t_log.empty())
						{
							std::cout << "GLSL warnings encountered:\n" << t_log << std::endl;
						}
					}
					
					auto handle() const
						-> GLuint
					{
						return m_Handle;
					}			
		
				private:
					// Opaque GL shader handle
					GLuint m_Handle;
			};
		}
		
		constexpr internal::from_file_t from_file{ };
		constexpr internal::from_text_t from_text{ };
		
		using fragment_shader = internal::shader<internal::shader_type::fragment>;
		using vertex_shader = internal::shader<internal::shader_type::vertex>;
		using compute_shader = internal::shader<internal::shader_type::compute>;
		using geometry_shader = internal::shader<internal::shader_type::geometry>;
		using tess_control_shader = internal::shader<internal::shader_type::tess_control>;
		using tess_evaluation_shader = internal::shader<internal::shader_type::tess_evaluation>;
		
	}
}
