#pragma once

#include <string>
#include <type_traits>
#include <stdexcept>
#include <glm/glm.hpp>
#include <ut/type_traits.hxx>
#include <ut/throwf.hxx>

namespace internal
{
	struct shadow_texture_t
	{
		::std::string m_Path;
	};

	struct text_texture_t
	{
		::std::string m_Path;
	};

	struct graphics_texture_t
	{
		::std::string m_Path;
	};

	template< typename T >
	using is_texture_tag = 	::std::disjunction<
								::std::is_same<::std::decay_t<T>, shadow_texture_t>,
								::std::is_same<::std::decay_t<T>, text_texture_t>,
								::std::is_same<::std::decay_t<T>, graphics_texture_t>
							>;
						
	template< typename T >
	constexpr bool is_texture_tag_v = is_texture_tag<T>::value;

	template< typename T, typename... Ts >
	using all_unique_impl = ::std::bool_constant<ut::count_of_v<T, Ts...> <= 1U>;

	template< typename... Ts >
	using all_unique = 	::std::conjunction<
							all_unique_impl<Ts, Ts...>...
						>;							
}


internal::shadow_texture_t shadow_texture(const ::std::string&);
internal::text_texture_t text_texture(const ::std::string&);
internal::graphics_texture_t graphics_texture(const ::std::string&);


// A class managing the ascii glyph sheet
class texture_set
{
	using size_type = ::std::size_t;
	using dimension_type = glm::ivec2;

	public:
		// Dimensions of glyph sheet, in glyphs
		const static size_type sheet_width = 16;
		const static size_type sheet_height = 16;
		
	public:
		template<	typename... Ts,
					typename = ::std::enable_if_t<
						sizeof...(Ts) <= 3U &&
						::std::conjunction_v<
							internal::is_texture_tag<::std::decay_t<Ts>>...,
							internal::all_unique<::std::decay_t<Ts>...>
						>
					>
		>
		texture_set(Ts&&... p_args)
		{
			static_assert(ut::contains_v<internal::text_texture_t, ::std::decay_t<Ts>...>,
				"texture_set: text texture required but not supplied");
			
			static_assert(ut::contains_v<internal::shadow_texture_t, ::std::decay_t<Ts>...>,
				"texture_set: shadow texture required but not supplied");
			
			glEnable(GL_TEXTURE_2D);
			
			auto t_x = { (dispatch(p_args), 0)... }; 
			(void)t_x;
			
			use();
		}
		
		texture_set() = default;
		~texture_set();
		
		texture_set(texture_set&&);
		texture_set(const texture_set&) = delete;
		
		texture_set& operator=(texture_set&&);
		texture_set& operator=(const texture_set&) = delete;

	private:
		void dispatch(const internal::shadow_texture_t&);
		void dispatch(const internal::text_texture_t&);
		void dispatch(const internal::graphics_texture_t&);
		
	private:
		auto recv_dims(GLuint p_tex)
			-> dimension_type;
			
		auto update_dims(const dimension_type&)
			-> void;

	public:
		// Returns dimensions of a single glyph, in pixels
		auto glyph_size() const
			-> const dimension_type&;
			
		auto use() const
			-> void;

	private:
		GLuint m_TextTex{0};
		GLuint m_GfxTex{0};
		GLuint m_ShadowTex{0};
		dimension_type m_GlyphDim{};	
};
