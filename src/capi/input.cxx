#include <capi/input.h>

#include <global_state.hxx>

extern "C"
{
	void input_begin()
	{
		global_state<input_manager>().begin_input();
	}
	
	void input_end()
	{
		global_state<input_manager>().end_input();
	}
	
	bool_t input_has_key(int p_key)
	{
		return static_cast<bool_t>(global_state<input_manager>().has_key(p_key));
	}
}
