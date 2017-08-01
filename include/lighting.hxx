// TODO: light_wrapper: On construction creates light and stores handle. When dereferenced, calls modify_light.
//		   				On destruction calls destroy_light.

#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <GLXW/glxw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using gpu_bool = ::std::uint32_t;

struct lighting_state
{
	gpu_bool m_UseLighting{true};
	gpu_bool m_UseDynamic{true};		
	glm::vec2 m_TlPositon{0.f, 0.f};
	glm::vec4 m_AmbientLight{1.f};
};

struct light
{
	glm::ivec2 m_Position;
	float m_Intensity;
	gpu_bool m_UseRadius;
	glm::vec4 m_Color;
	glm::vec3 m_AttFactors;
	float m_Radius;
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
	
	static_assert(sizeof(light) == light_size, "size of struct light does not match light_size");
	static_assert(sizeof(lighting_state) == state_size, "size of struct lighting_state does not match state_size");
	
	public:
		using handle_type = ::std::size_t;
		using size_type = ::std::uint32_t;
	
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
		bool has_space(::std::size_t p_amount = 1U) const;
		
	private:
		// Check if handle is in bounds
		bool check_handle(handle_type) const;
	
	private:
		bool m_Dirty{true}; 						//< Whether the data was modified this frame
		unsigned m_GPUBuffer;						//< Handle of GPU Buffer
		
		size_type m_LightCount{0U};					//< Current number of lights
		::std::array<bool, max_lights> m_Used{false};	//< Contains info about which entries are used
		::std::array<light, max_lights> m_Lights; 		//< Light data
		lighting_state m_State;							//< Lighting state
};
