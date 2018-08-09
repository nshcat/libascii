#include <capi/renderer.h>

#include <global_state.hxx>

extern "C"
{
	void renderer_render()
	{
		global_state<render_manager>().render();
	}	
}
