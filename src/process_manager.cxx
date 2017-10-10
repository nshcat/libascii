#include <stdexcept>
#include <ut/small_vector.hxx>

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

auto process_manager::kill_process(process_id p_id)
	-> void
{
	// Validate given PID
	if(p_id == no_process)
		return;
		
	// Try to find process entry
	const auto t_it = m_ProcMap.find(p_id);
	if(t_it != m_ProcMap.end())
	{
		// Process entry was found. Retrieve process type to decide
		// in what auxiliary data structures views on this process
		// are stored
		auto& t_procList = proc_list(t_it->second->type());
		
		// Find position
		const auto t_pos = binary_find(::std::begin(t_procList), ::std::end(t_procList), t_it->second,
			[](const auto& p_l, const auto& p_r) -> bool
			{
				return ut::enum_cast(p_l->priority()) < ut::enum_cast(p_r->priority());
			}
		);
		
		// Erase element
		t_procList.erase(t_pos);
		
		// Remove process entry from main map
		m_ProcMap.erase(p_id);
	}

	// Process is either now known or was deleted, work is done.
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
			case process_state::active:
			{
				// If the process was already active, it needs to be put to sleep if it has the
				// "periodic_sleep" flag.
				if(t_procView->flags() & process_flags::periodic_sleep)
				{
					// Sanity check: Are we even trying to sleep?
					if(t_procView->periodic_duration() != no_sleep)
					{
						// Since the process is not executed this time slice, it needs to sleep for
						// one less time slice than requested
						t_procView->set_state(process_state::sleeping);
						t_procView->set_sleep_duration(t_procView->periodic_duration() - 1U);
					}
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
	
	// Buffer for PIDs of processes that have exceeded their runtime
	// limits and thus need to be killed. Removal of these processes
	// has to be postponed to after updating is done, since iterators
	// would be invalidated otherwise.
	ut::small_vector<process_id, 16> t_pidsToKill{ };
	
	// Iterate through processes of matching type, from high to
	// low priority. (hight priority is a low numerical value)
	for(auto t_procView: t_procList)
	{
		// Only give time slice to processes that are in active
		// state
		if(t_procView->state() == process_state::active)
		{
			t_procView->update();
			
			// Increment runtime counter
			t_procView->inc_runtime();
			
			// If the process has a runtime limit, check if it was
			// exceeded. In that case, the process needs to be added
			// to list to be killed later
			if(t_procView->flags() & process_flags::limited_runtime)
			{
				if(t_procView->runtime() >= t_procView->runtime_limit())
					t_pidsToKill.push_back(t_procView->pid());
			}
		}
	}
	
	// Kill all processes stored in the to-kill list
	for(const auto& t_pid: t_pidsToKill)
		kill_process(t_pid);
}

auto process_manager::proc_list(process_type p_type)
	-> process_list&
{
	return ((p_type == process_type::per_frame) ?
			m_PerFrameProcs : m_PerTickProcs);
}

auto process_manager::begin()
	-> iterator
{
	return { m_ProcMap.begin(), { } };
}

auto process_manager::end()
	-> iterator
{
	return { m_ProcMap.end(), { } };
}
