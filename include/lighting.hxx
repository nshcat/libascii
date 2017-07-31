#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


struct lighting_state
{
	
};

struct light
{
};


// Maintains lighting state and container of lights.
// On every frame, sync() is called. If any of the data changed, the
// GPU buffer is updated. TODO use SubBufferData to only update parts
// of the buffer
// TODO use different dirty bits to only update whats necessary
class light_manager
{
	static constexpr ::std::size_t max_lights = 25;
	static constexpr ::std::size_t light_size = 8 + 4 + 4 + 16 + 12 + 4;
	static constexpr ::std::size_t count_size = 4;
	static constexpr ::std::size_t state_size = 16 + 16;
	static constexpr ::std::size_t buffer_size = state_size + (max_lights * light_size) + count_size;
	
	public:
		using handle_type = ::std::size_t;
	
	public:
		light_manager();
		
	public:
		light_manager(const light_manager&) = delete;
		light_manager(light_manager&&) = delete;
		
		light_manager& operator=(const light_manager&) = delete;
		light_manager& operator=(light_manager&&) = delete;
	
	public:
		// Sync buffer on GPU with state contained in this object
		void sync();
		
		// Create new light from given template and return handle.
		// Will throw if no space is left.
		handle_type create_light(const light& p_light);
		
		// Retrieves temporary reference to stored light with given
		// handle. Using this method signals the implementation
		// to assume that the light has been changed.
		// Will throw if there is no light with given handle.
		light& modify_light(handle_type);
		
		// Destroy light with given handle.
		// This will throw if there is not light with given handle.
		void destroy_light(handle_type);
		
		// Retrieve temporary reference to stored lighting state.
		// Using this method signals the implementation to assume
		// that the state has been changed.
		lighting_state& modify_state();
		
		// Checks if there is enough space left for N lights
		bool has_space(::std::size_t p_amount = 1U);
	
	private:
		bool m_Dirty{true}; 						//< Whether the data was modified this frame
		unsigned m_GPUBuffer;						//< Handle of GPU Buffer
		
		::std::array<bool, max_lights> m_Used;		//< Contains info about which entries are used
		::std::array<light, max_lights> m_Lights; 	//< Light data
		lighting_state m_State;						//< Lighting state
};
