#include <capi/lighting.h>
#include <global_state.hxx>

extern "C"
{
	uint64_t lighting_create_light(light_t* p_light)
	{
		return global_state<light_manager>().create_light(*reinterpret_cast<light*>(p_light));
	}
	
	void lighting_destroy_light(uint64_t p_handle)
	{
		global_state<light_manager>().destroy_light(p_handle);
	}
	
	bool_t lighting_has_space(int p_count)
	{
		return static_cast<bool_t>(global_state<light_manager>().has_space(p_count));
	}
	
	void lighting_set_ambient(uvec3_t* p_clr)
	{
		glm::vec4 nativeClr{ p_clr->r / 255.f, p_clr->g / 255.f, p_clr->b / 255.f, 1.f };
		global_state<light_manager>().modify_state().m_AmbientLight = nativeClr;
	}
}

