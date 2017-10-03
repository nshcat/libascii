#include <global_state.hxx>

auto global_state_t::context()
	-> render_context&
{
	return m_Context;
}

auto global_state_t::path_manager()
	-> class path_manager&
{
	return m_PathManager;
}

auto global_state_t::process_manager()
	-> class process_manager&
{
	return m_ProcessManager;
}

auto global_state_t::asset_manager()
	-> class asset_manager&
{
	return m_AssetManager;
}

auto global_state_t::configuration()
	-> class configuration&
{
	return m_Configuration;
}

auto global_state_t::initialize()
	-> void
{
	m_PathManager.initialize();
	m_Configuration.initialize();
	m_ProcessManager.initialize();
	m_Context.initialize();
}

auto global_state()
	-> global_state_t&
{
	// Threadsafe, global instance
	static global_state_t g_GlobalState{ };
	
	return g_GlobalState;
}
