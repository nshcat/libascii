#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// Is supposed to work similar to the light_manager.
// But using glBufferSubData for every entry could be very problematic here.
// Find solution!

// SOLUTION: struct `cell` that is two uvec4, like in the buffer.
// But the struct has methods to modify it, like set/get fore/background color, and bitfields..
class screen_manager
{
	using position_type = glm::uvec2;
	using dimension_type = glm::uvec2;

	public:
		screen_manager(dimension_type p_screenSize);
		
	public:
		screen_manager(const light_manager&) = delete;
		screen_manager(light_manager&&) = delete;
		
		screen_manager& operator=(const screen_manager&) = delete;
		screen_manager& operator=(screen_manager&&) = delete;
	
	public:
		// Sync buffer on GPU with state contained in this object
		void sync();
		
		void clear_screen();
		void clear_cell(position_type);
		cell& modify_cell(position_type);
		void set_cell(position_type);	
	
	private:
		bool m_Dirty{true}; 						//< Whether the data was modified this frame
		unsigned m_GPUBuffer;						//< Handle of GPU Buffer
		dimension_type m_ScreenDims;				//< Dimensions of screen, in glyphs
};
