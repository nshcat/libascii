#pragma once

#include <GLXW/glxw.h>

namespace gl
{
	// A class that manages the life time of an object created by the OpenGL runtime.
	// A custom deleter type is used to destroy the object when needed.
	// This class can either be used as a RAII wrapper, or by using initialize()/deinitialize().
	// This functionality is sometimes needed when objects of this type are used in class that need
	// to have a trivial default constructor and destructor, like the classes used as global systems.
	// TODO accept deleter in constructor and save it in member, this way using deduction guides, lambdas
	// can be used. In its current form, the deleter is created in various functions, which is not possible
	// with lambdas!
	// TODO constructors, and destructors. Destructor is not called if UseRAII is false.
	// reset and clear can be used as initialize/deinitialize
	template< typename TDeleter, bool UseRAII = true >
	class unique_resource
	{
		using this_type = unique_resource<TDeleter, UseRAII>;
	
		public:
			using handle_type = GLuint;	//< Type used by OpenGL to represent handles to objects
	
		public:
			constexpr static handle_type invalid_handle = { }; // TODO is this correct for all handles?
	
		public: // Observers
			auto get() const noexcept
				-> handle_type
			{
				return m_Handle;
			}
			
			explicit operator bool() () const noexcept
			{
				return m_Handle != invalid_handle;
			}
			
		public: // Modifiers
			auto release() noexcept
				-> handle_type
			{
				const auto t_tmp = get();
				m_Handle = invalid_handle;
				return t_tmp;
			}
			
			auto reset(handle_type t_handle)
				-> void
			{
				const auto t_tmp = get();
				m_Handle = t_handle;
				if(t_tmp != invalid_handle)
				{
					TDeleter t_del{ };
					t_del(t_tmp);
				}
			}
			
			auto clear()
				-> void
			{
				reset(invalid_handle);
			}
		
			auto swap(this_type& p_other) noexcept
				-> void
			{
				using ::std::swap;
				swap(m_Handle, p_other.m_Handle);
			}
		
		private:
			handle_type m_Handle{ invalid_handle };	//< A value of invalid_handle is interpreted as "empty"
	};
}

namespace std
{
	template< typename T, bool R >
	auto swap(gl::unique_resource<T, R>& p_lhs, gl::unique_resource<T, R>& p_rhs) noexcept
		-> void
	{
		p_lhs.swap(p_rhs);
	}
	
	template< typename T, bool R >
	struct hash <gl::unique_resource<T, R>>
	{
		using argument_type = gl::unique_resource<T, R>;
		
		auto operator()(const argument_type& p_res)
			-> ::std::size_t
		{
			return ::std::hash<typename argument_type::handle_type>{ }(p_res.get());
		}
	};
}
