#include <stdexcept>

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

auto process_manager::register_process(process_ptr p_proc)
	-> process_view
{
	// Create view from unique_ptr to save a lookup at the end
	process_view t_view = process_view{ p_proc.get() };
	
	// Insert process into lookup map
	m_ProcMap.emplace(t_view->pid(), ::std::move(p_proc));
	
	// Insert process view into appropiate list
	insert_sorted((t_view->type() == process_type::per_frame) ? m_PerFrameProcs : m_PerTickProcs, t_view,
		[](const auto& p_l, const auto& p_r) -> bool
		{
			return ut::enum_cast(p_l->priority()) < ut::enum_cast(p_r->priority());
		}
	);
	
	// Initialize process
	t_view->initialize();
	
	// If process state was not changed, change it to active.
	// This allows initialize() to switch the process into waiting or paused state
	// without us overwriting that here
	if(t_view->state() == process_state::inactive)
		t_view->set_state(process_state::active);
		
	return t_view;
}

auto process_manager::get_state(process_id p_id) const
	-> process_state
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

auto process_manager::get_process(process_id p_id)
	-> process_view
{
	// Try to find process with given id
	const auto t_it = m_ProcMap.find(p_id);
	
	if(t_it != ::std::end(m_ProcMap))
		return { t_it->second.get() };
	else throw ::std::runtime_error("process_manager: tried process with given pid not found");
}

auto process_manager::next_pid()
	-> process_id
{
	// TODO better strategy (see notes)
	return m_NextPid++;
}

auto process_manager::kill_process(process_id)
	-> void
{
	return;
}

auto process_manager::update_states(process_type p_type)
	-> void
{
	// Determine process list that should be used
	auto& t_procList = proc_list(p_type);
						
	// Iterate and check state.
	for(auto& t_procView: t_procList)
	{
		switch(t_procView->state())
		{
			case process_state::sleeping:
			{
				// Process is currently sleeping. Check if it should be switched
				// to active by now.
				if(t_procView->sleep_duration() == no_sleep)
				{
					// Process is done sleeping. Switch to active state.
					t_procView->set_state(process_state::active);
				}
				else
				{
					// Process still needs to sleep. Decrement remaining sleep duration.
					t_procView->dec_sleep_duration();
				}
				
				break;
			}
			case process_state::waiting:
			{
				// Process is waiting on something. Check if that precondition is
				// fulfilled by now.
				if(const auto t_state = get_state(t_procView->wait_pid());
					t_state == process_state::dead || t_state == process_state::nonexistent)
				{
					// Condition is fulfilled, process can continue running.
					t_procView->set_state(process_state::active);
					t_procView->set_wait_pid(no_process);
				}
				
				break;
			}
			default:
			{
				// Do nothing for processes in other states.
				break;
			}
		}
	}
}

auto process_manager::update_processes(process_type p_type)
	-> void
{
	// Update process states
	update_states(p_type);
	
	// Determine which process list to use
	auto& t_procList = proc_list(p_type);
	
	// Iterate through processes of matching type, from high to
	// low priority. (hight priority is a low numerical value)
	for(auto t_procView: t_procList)
	{
		// Only give time slice to processes that are in active
		// state
		if(t_procView->state() == process_state::active)
			t_procView->update();
	}
}

auto process_manager::proc_list(process_type p_type)
	-> process_list&
{
	return ((p_type == process_type::per_frame) ?
			m_PerFrameProcs : m_PerTickProcs);
}


