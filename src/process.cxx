#include <process.hxx>
#include <global_state.hxx>

process::process(process_id p_id, process_id p_parent, process_type p_type, process_priority p_prio)
	: m_Pid{p_id}, m_Parent{p_parent}, m_Priority{p_prio}, m_Type{p_type}
{
}

auto process::kill()
	-> void
{
	// Killing a process has to be delegated to the scheduler,
	// since it involves changing global data structures
	global_state().process_manager().kill_process(pid());
}

auto process::wait_for(process_id p_id)
	-> void
{
	// Skip the check for obviously nonexistent processes
	if(p_id == no_process)
		return;

	// We should not wait for ourselves, deadlock would be
	// imminent ;)
	if(p_id == pid())
		return;
		
	// If the other process is already dead or nonexistent,
	// this operation is a nop.
	if(const auto t_state = global_state().process_manager().get_state(p_id);
		t_state == process_state::nonexistent || t_state == process_state::dead)
	{
		return;
	}
		
	// If we are currently sleeping, the remaining sleep duration
	// will be overridden, since waiting for a condition has priority
	if(sleep_duration() != no_sleep)
		set_sleep_duration(no_sleep);
		
	// Save pid of other process and switch state
	set_wait_pid(p_id);
	set_state(process_state::waiting);
}

auto process::sleep(::std::size_t p_duration)
	-> void
{
	// We can't make a waiting process sleep, since
	// waiting is a condition that has priority
	if(state() == process_state::waiting)
		return;

	// If we are already sleeping or zero sleep duration was
	// supplied, there is nothing to do here
	if(state() == process_state::sleeping
		|| p_duration == no_sleep)
	{
		return;
	}

	// Set sleep duration
	set_sleep_duration(p_duration);
	
	// Change state
	set_state(process_state::sleeping);
}

auto process::periodic_sleep(::std::size_t p_duration, bool p_initialSleep)
	-> void
{
	// No sleep duration is invalid here
	if(p_duration == no_sleep)
		return;
		
	set_flags(flags() | process_flags::periodic_sleep);
	set_periodic_duration(p_duration);
	
	// Only switch state to sleeping if requested by user
	set_state(process_state::sleeping);
	// This is a hack to allow the process to immediately run if requested
	set_sleep_duration(p_initialSleep ? p_duration : no_sleep);
}

auto process::pid() const
	-> process_id
{
	return m_Pid;
}

auto process::parent() const
	-> process_id
{
	return m_Parent;
}

auto process::state() const
	-> process_state
{
	return m_State;
}

auto process::type() const
	-> process_type
{
	return m_Type;
}

auto process::flags() const
	-> process_flags
{
	return m_Flags;
}

auto process::wait_pid() const
	-> process_id
{
	return m_WaitPid;
}

auto process::priority() const
	-> process_priority
{
	return m_Priority;
}

auto process::runtime() const
	-> ::std::size_t
{
	return m_Runtime;
}

auto process::runtime_limit() const
	-> ::std::size_t
{
	return m_RuntimeLimit;
}

auto process::periodic_duration() const
	-> ::std::size_t
{
	return m_PeriodicDuration;
}

auto process::set_periodic_duration(::std::size_t p_duration)
	-> void
{
	m_PeriodicDuration = p_duration;
}

auto process::sleep_duration() const
	-> ::std::size_t
{
	return m_SleepDuration;
}

auto process::set_sleep_duration(::std::size_t p_duration)
	-> void
{
	m_SleepDuration = p_duration;
}

auto process::set_runtime_limit(::std::size_t p_limit)
	-> void
{
	m_RuntimeLimit = p_limit;
}

auto process::set_wait_pid(process_id p_id)
	-> void
{
	m_WaitPid = p_id;
}

auto process::kill_after(::std::size_t p_limit)
	-> void
{
	if(p_limit == no_limit)
		return;

	set_runtime_limit(p_limit);
	set_flags(flags() | process_flags::limited_runtime);
}

auto process::dec_sleep_duration()
	-> void
{
	--m_SleepDuration;
}

auto process::inc_runtime()
	-> void
{
	++m_Runtime;
}

auto process::set_parent(process_id p_id)
	-> void
{
	m_Parent = p_id;
}

auto process::set_flags(process_flags p_flags)
	-> void
{
	m_Flags = p_flags;
}

auto process::set_state(process_state p_state)
	-> void
{
	m_State = p_state;
}

