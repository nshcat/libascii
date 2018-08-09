#include <capi/render_context.h>
#include <global_state.hxx>

extern "C"
{
	bool_t render_context_should_close()
	{
		return static_cast<bool_t>(global_state<render_context>().should_close());
	}
	
	void render_context_begin_frame()
	{
		global_state<render_context>().begin_frame();
	}
	
	void render_context_end_frame()
	{
		global_state<render_context>().end_frame();
	}
}
