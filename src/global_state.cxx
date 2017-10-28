#include <global_state.hxx>

auto global_state()
	-> global_state_t&
{
	// Threadsafe, global instance
	static global_state_t g_GlobalState{ };
	
	return g_GlobalState;
}
