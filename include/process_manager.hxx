#pragma once

#include <memory>
#include <utility>
#include <unordered_map>
#include <type_traits>
#include <ut/observer_ptr.hxx>
#include <ut/cast.hxx>

#include "global_system.hxx"
#include "process.hxx"
#include "utility.hxx"


// Problem: If a parent process gets deleted, that has a child that waits for it, but the PID
// is reused before the scheduler checks the WAITING condition, the child stays in waiting condition!
// => Processes cannot easily be reused!
// Fix 1): To use increasing, unique ids. This would work and be the pragmatic solution, but fuck that
// Fix 2): Implement proper garbage collection using a background process that regulary checks the
//		   dependency chain and removes the processes.



class process_manager
	: public global_system
{
	using process_ptr = ::std::unique_ptr<process>;
	using process_view = ut::observer_ptr<process>;
	using process_map = ::std::unordered_map<process_id, process_ptr>;
	using process_list = ::std::vector<process_view>;
	
	// Allow process base class methods to access housekeeping methods
	friend class process;
	
	public:
		auto initialize()
			-> void;
			
		auto frame()
			-> void;
			
		auto tick()
			-> void;
		
	public:
		// TODO put as much as possible of this method into a
		// non-template method. (one that takes unique_ptr<process>)
		template< typename T, typename... Ts >
		auto create_process(process_id p_parent, Ts&&... p_args)
			-> process_view
		{
			static_assert(::std::is_base_of_v<process, T>,
				"T needs to be derived from process!");
		
			// Create new process object and register it
			return register_process(
				::std::make_unique<T>(next_pid(), p_parent, ::std::forward<Ts>(p_args)...)
			);
		}
		
	public:
		auto kill_process(process_id)	
			-> void;
				
		auto get_state(process_id) const
			-> process_state;
			
		auto get_process(process_id);
			-> process_view;	
			
	private:
		// TODO really return view here? The view can be constructed
		// in create_process aswell.
		auto register_process(process_ptr)
			-> process_view;
	
		auto next_pid()
			-> process_id;
		
		// Assigns every process of the given type
		// a time slice.
		auto update_processes(process_type)
			-> void;
			
		// Updates the states of processes. Some processes
		// might require a state change because of fulfilled
		// conditions etc.
		auto update_states(process_type)
			-> void;
			
		auto proc_list(process_type)
			-> process_list&;
			
	private:
		process_id	m_NextPid{1U};		//< The next pid that may be used for a process
		process_map m_ProcMap;			//< Fast lookup map that owns all processes
		process_list m_PerFrameProcs;	//< Ordered (by priority) list of process views (per frame)
		process_list m_PerTickProcs;	//< Ordered (by priority) list of process views (per tick)
};
