#pragma once

#include <stdexcept>
#include <utility>
#include <GLXW/glxw.h>

#include <ut/array_view.hxx>
#include <ut/string_view.hxx>
#include "program.hxx"

namespace ut
{
	namespace gl
	{
		enum class uniform_type
			: ::std::size_t
		{
			mat2 = 0,
			mat3,
			mat4,
			vec2,
			vec3,
			vec4,
			end_marker
		};
		
		namespace internal
		{
			template< typename T >
			struct type_holder
			{
				using type = T;
			};
			
			template< uniform_type Type >
			struct uniform_base_type;
			
			#define LIBUT_GL_MKTYPE( __ENUMVAR, __UBTYPE ) 			\
				template< >											\
				struct uniform_base_type <uniform_type::__ENUMVAR >	\
					: public type_holder<__UBTYPE>					\
				{};						
				
			LIBUT_GL_MKTYPE(mat2, GLfloat)
			LIBUT_GL_MKTYPE(mat3, GLfloat)
			LIBUT_GL_MKTYPE(mat4, GLfloat)
			LIBUT_GL_MKTYPE(vec2, GLfloat)
			LIBUT_GL_MKTYPE(vec3, GLfloat)
			LIBUT_GL_MKTYPE(vec4, GLfloat)
				
			#undef LIBUT_GL_MKTYPE
			
			template< uniform_type Type >
			using uniform_base_type_t = typename uniform_base_type<Type>::type;
			
			
			// Size of uniform in base_type s
			template< uniform_type Type >
			constexpr auto uniform_size()
				-> ::std::size_t
			{
				constexpr ::std::size_t t_table[] =
				{
					2*2,
					3*3,
					4*4,
					2,
					3,
					4
				};
				
				return t_table[ static_cast<::std::size_t>(Type) ];
			}
		}
		
		// 
		namespace internal
		{
			#define LIBUT_GL_MKDISPATCH( __ENUMVAR ) 				\
			template< typename T >									\
			auto set_uniform_impl(	::std::integral_constant<		\
										uniform_type,				\
										uniform_type::__ENUMVAR		\
									>,								\
									GLint p_location,				\
									array_view<const T> p_data,		\
									::std::size_t p_count			\
								)									\
				-> void		
				
				
				
			LIBUT_GL_MKDISPATCH(mat3)
			{
				glUniformMatrix3fv(	p_location,
									p_count,
									GL_FALSE,
									static_cast<const GLfloat*>(
										p_data.data()
									)
								);
			}
			
			LIBUT_GL_MKDISPATCH(mat4)
			{
				glUniformMatrix4fv(	p_location,
									p_count,
									GL_FALSE,
									static_cast<const GLfloat*>(
										p_data.data()
									)
								);
			}
			
			LIBUT_GL_MKDISPATCH(mat2)
			{
				glUniformMatrix2fv(	p_location,
									p_count,
									GL_FALSE,
									static_cast<const GLfloat*>(
										p_data.data()
									)
								);
			}
			
			LIBUT_GL_MKDISPATCH(vec2)
			{
				glUniform2fv(	p_location,
								p_count,
								static_cast<const GLfloat*>(
									p_data.data()
								)
							);
			}
			
			LIBUT_GL_MKDISPATCH(vec3)
			{
				glUniform3fv(	p_location,
								p_count,
								static_cast<const GLfloat*>(
									p_data.data()
								)
							);
			}
			
			LIBUT_GL_MKDISPATCH(vec4)
			{
				glUniform4fv(	p_location,
								p_count,
								static_cast<const GLfloat*>(
									p_data.data()
								)
							);
			}
			
			#undef LIBUT_GL_MKDISPATCH

		}
		
		/*
		const program& p_prog,
							string_view p_name,
		*/
		
		template< uniform_type Type >
		auto set_uniform(	const program& p_prog,
							string_view p_name,
							array_view< ::std::add_const_t<internal::uniform_base_type_t<Type>> >
								p_data,
							::std::size_t p_count
						)
			-> void
		{
			// Get location. We need a zero-terminated string here.
			auto t_str = p_name.to_string();
			auto t_loc = glGetUniformLocation(p_prog.handle(), t_str.c_str());
			
			if(t_loc != -1)
			{
				set_uniform<Type>(t_loc, p_data, p_count);
			}
			else throw ::std::runtime_error("Invalid uniform name for given program!");
		}
		
		template< uniform_type Type >
		auto set_uniform(	GLint p_location,
							array_view< ::std::add_const_t<internal::uniform_base_type_t<Type>> >
								p_data,
							::std::size_t p_count
						)
			-> void
		{
			// uniform_type::end_marker is obviously not allowed
			static_assert(Type != uniform_type::end_marker,
				"end_marker is not a valid uniform type!");
		
			// Assert that size is correct
			if( p_data.size() < internal::uniform_size<Type>() * p_count )
				throw ::std::runtime_error("set_uniform: Provided buffer not long enough!");
				
			// Call matching implementation using tag dispatching
			using Tdispatch = ::std::integral_constant<uniform_type, Type>;
			using Tbase = internal::uniform_base_type_t<Type>;
			
			internal::set_uniform_impl<Tbase>( Tdispatch{}, p_location, p_data, p_count );
		}
	
	}
			
}
