#include <process_manager.hxx>

auto process_manager::initialize()
	-> void
{
	return;
}

auto process_manager::frame()
	-> void
{
	update_processes(process_type::per_frame);
}

auto process_manager::tick()
	-> void
{
	update_processes(process_type::per_tick);
}

auto process_manager::process_state(process_id p_id) const
	-> class process_state
{
	// A process with pid 0 cannot exist
	if(p_id == no_process)
		return process_state::nonexistent;

	// Try to find process with given id
	const auto t_it = m_ProcMap.find(p_id);
	
	if(t_it != ::std::end(m_ProcMap))
		return t_it->second->state();
	else
		return process_state::nonexistent;
}
