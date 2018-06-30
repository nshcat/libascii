#include <engine.hxx>
#include <global_state.hxx>

auto engine::initialize(const game_info& p_info, int p_argc, const char** p_argv)
	-> void
{
	// Store command line arguments for later use
	populate_argv(p_argc, p_argv);
	
	// Register game information with the command line handler
	populate_info(p_info);
	
	// Init global state
	global_state().initialize();	
}
