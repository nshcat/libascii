#pragma once

#include <stdexcept>
#include <utility>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ut/array_view.hxx>
#include <ut/string_view.hxx>
#include <ut/always_false.hxx>
#include "program.hxx"


namespace gl
{
	namespace internal
	{
		template< typename T >
		auto set_uniform_impl(GLint, const T&)
			-> void
		{
			static_assert(ut::always_false_v<T>, "Given type is not supported by set_uniform!");
		}
			
			
		#define MAKE_UNIFORM_IMPL(__type) 											\
		template< >																	\
		auto set_uniform_impl<__type>(GLint p_location, const __type& p_value) 		\
			-> void																
				
		#define MAKE_UNIFORM_IMPL_VEC(__type, __suffix)								\
		MAKE_UNIFORM_IMPL(__type)													\
		{																			\
			glUniform##__suffix ( p_location, 1, glm::value_ptr(p_value) );			\
		}
		
		#define MAKE_UNIFORM_IMPL_BVEC(__length)									\
		MAKE_UNIFORM_IMPL(glm::bvec##__length)										\
		{																			\
			glm::uvec##__length t_tmp(p_value);										\
			glUniform##__length##uiv ( p_location, 1, glm::value_ptr(t_tmp) );		\
		}
		
		#define MAKE_UNIFORM_IMPL_SQUARE_MAT(__type, __suffix)						\
		MAKE_UNIFORM_IMPL(__type)													\
		{																			\
			glUniform##__suffix ( p_location, 1, false, glm::value_ptr(p_value) );	\
		}
			
		#define MAKE_UNIFORM_IMPL_SCALAR(__type, __suffix)							\
		MAKE_UNIFORM_IMPL(__type)													\
		{																			\
			glUniform##__suffix ( p_location, p_value );							\
		}
		
		
		MAKE_UNIFORM_IMPL_VEC(glm::vec2, 2fv)
		MAKE_UNIFORM_IMPL_VEC(glm::vec3, 3fv)
		MAKE_UNIFORM_IMPL_VEC(glm::vec4, 4fv)
		
		MAKE_UNIFORM_IMPL_VEC(glm::ivec2, 2iv)
		MAKE_UNIFORM_IMPL_VEC(glm::ivec3, 3iv)
		MAKE_UNIFORM_IMPL_VEC(glm::ivec4, 4iv)
		
		MAKE_UNIFORM_IMPL_VEC(glm::uvec2, 2uiv)
		MAKE_UNIFORM_IMPL_VEC(glm::uvec3, 3uiv)
		MAKE_UNIFORM_IMPL_VEC(glm::uvec4, 4uiv)
		
		MAKE_UNIFORM_IMPL_BVEC(2)
		MAKE_UNIFORM_IMPL_BVEC(3)
		MAKE_UNIFORM_IMPL_BVEC(4)
		
		MAKE_UNIFORM_IMPL_SQUARE_MAT(glm::mat2, Matrix2fv)
		MAKE_UNIFORM_IMPL_SQUARE_MAT(glm::mat3, Matrix3fv)
		MAKE_UNIFORM_IMPL_SQUARE_MAT(glm::mat4, Matrix4fv)
		
		MAKE_UNIFORM_IMPL_SCALAR(unsigned int, 1ui)
		MAKE_UNIFORM_IMPL_SCALAR(int, 1i)
		MAKE_UNIFORM_IMPL_SCALAR(bool, 1ui)
		MAKE_UNIFORM_IMPL_SCALAR(float, 1f)
		
		#undef MAKE_UNIFORM_IMPL_VEC
		#undef MAKE_UNIFORM_IMPL
	}
	
	template< typename T >
	auto set_uniform(GLint p_location, const T& p_value)
		-> void
	{
		internal::set_uniform_impl<T>(p_location, p_value);
	}

	template< typename T >
	auto set_uniform(const program& p_prog, ut::string_view p_name, const T& p_value)
		-> void
	{
		// Get location. We need a zero-terminated string here.
		auto t_str = p_name.to_string();
		auto t_loc = glGetUniformLocation(p_prog.handle(), t_str.c_str());
	
		if(t_loc != -1)
		{
			set_uniform<T>(t_loc, p_value);
		}
		else throw ::std::runtime_error("Invalid uniform name for given program!");
	}
}
