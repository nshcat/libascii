#include <engine.hxx>
#include <global_state.hxx>

game_info engine::m_GameInfo;

auto engine::game_information()
	-> const game_info&
{
	return m_GameInfo;
}

auto engine::initialize(const game_info& p_info, int p_argc, const char** p_argv)
	-> void
{
	// Store command line arguments for later use
	populate_argv(p_argc, p_argv);
	
	// Register game information with the command line handler
	populate_info(p_info);
	
	// Save game information in static state of this class for use
	// by the global systems during initialization
	m_GameInfo = p_info;
	
	// Init global state
	global_state().initialize();	
}
