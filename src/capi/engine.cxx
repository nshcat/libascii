#include <capi/engine.h>
#include <engine.hxx>
#include <global_state.hxx>

extern "C"
{
	void engine_initialize(game_info_t* p_info, int p_argc, const char** p_argv)
	{
		// Translate to C++ game info class instance for use with initialization
		// methods
		game_info t_info{
			{ p_info->name },
			{ p_info->version },
			{ p_info->description },
			{ p_info->windowTitle }
		};
		
		engine::initialize(t_info, p_argc, p_argv);
	}
	
	void engine_deinitialize()
	{
		global_state<render_context>().deinitialize();
	}
}
