// TODO Make position_type arguments take them by reference

#pragma once

#include <type_traits>
#include <vector>
#include <optional>
#include <cstdint>
#include <array>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ut/cast.hxx>

// Is supposed to work similar to the light_manager.
// But using glBufferSubData for every entry could be very problematic here.
// Find solution!

// SOLUTION: struct `cell` that is two uvec4, like in the buffer.
// But the struct has methods to modify it, like set/get fore/background color, and bitfields..


enum class drop_shadow
	: ::std::uint32_t
{
	north 		 = 0x1U << 8U,
	west 		 = 0x1U << 9U,
	south 		 = 0x1U << 10U,
	east 		 = 0x1U << 11U,
	top_left 	 = 0x1U << 12U,
	top_right 	 = 0x1U << 13U,
	bottom_left  = 0x1U << 14U,
	bottom_right = 0x1U << 15U,
};

enum class light_mode
	: ::std::uint32_t
{
	none = 0U,
	dim  = 1U,
	full = 2U
};


// Bitmasks etc are only needed internally
namespace internal
{
	constexpr const ::std::uint32_t drop_shadow_mask = 0xFF00U;
	constexpr const ::std::uint32_t light_mode_mask = 0xF0000U;
	constexpr const ::std::uint32_t light_mode_shift = 16U;
	constexpr const ::std::uint32_t gui_mode_bit = 0x1U << 20U;
	constexpr const ::std::uint32_t fog_mask = 0xFFU;
}


struct cell
{
	using integral_color_type = glm::uvec3;
	using float_color_type = glm::vec3;
	using glyph_type = ::std::uint32_t;

	public:
		void set_fg(const integral_color_type&);
		void set_fg(const float_color_type&);
		
		const integral_color_type& fg() const;
		
		void set_bg(const integral_color_type&);
		void set_bg(const float_color_type&);
		
		const integral_color_type& bg() const;
		
		void set_glyph(glyph_type);
		glyph_type glyph() const;

	private:
		glm::uvec3 		m_Front{0, 0, 0};
		glyph_type		m_Glyph{0};
		glm::uvec3 		m_Back{0, 0, 0};
		::std::uint32_t m_Data{0};
};

static_assert(sizeof(cell) == 32, "cell struct size mismatch!");	
static_assert(alignof(cell) == 4, "cell struct alignment mismatch!");	


class screen_manager
{
	public:
		using position_type = glm::uvec2;
		using dimension_type = glm::uvec2;
		using index_type = ::std::size_t;
		using container_type = ::std::vector<cell>;

	public:
		screen_manager(dimension_type p_screenSize);
		
	public:
		screen_manager(const screen_manager&) = delete;
		screen_manager(screen_manager&&) = delete;
		
		screen_manager& operator=(const screen_manager&) = delete;
		screen_manager& operator=(screen_manager&&) = delete;
	
	public:
		// Sync buffer on GPU with state contained in this object
		void sync();
		
		void clear_screen();
		void clear_cell(position_type);
		cell& modify_cell(position_type);
		const cell& read_cell(position_type) const;
		void set_cell(position_type, const cell&);
		
	public:
		// TODO maybe make Taction a callable that accepts cell AND count
		// That would allow much more interesting actions
		template< typename Tshape, typename Taction >
		void modify(Tshape&& p_shape, Taction&& p_action)
		{
			Tshape t_shape{ ::std::forward<Tshape>(p_shape) };
		
			::std::optional<position_type> t_next;
			while(t_next = t_shape.next())
			{
				p_action(modify_cell(t_next.value()));
			}
			
			set_dirty();
		}
		
	private:
		index_type calc_index(position_type) const;
		void clear_cell(index_type);
		cell& get_cell(index_type);
		const cell& get_cell(index_type) const;
		void set_dirty();
		bool check_position(position_type) const;
	
	private:
		bool m_Dirty{false}; 						//< Whether the data was modified this frame
		GLuint m_GPUBuffer;							//< Handle of GPU Buffer
		GLuint m_GPUTexture;						//< Handle of the GPU texture
		dimension_type m_ScreenDims;				//< Dimensions of screen, in glyphs
		container_type m_Data;						//< Actual screen data
};
