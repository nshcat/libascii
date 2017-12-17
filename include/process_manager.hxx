#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <ut/observer_ptr.hxx>
#include <ut/cast.hxx>
#include <ut/small_vector.hxx>
#include <boost/iterator/transform_iterator.hpp>

#include "global_system.hxx"
#include "process.hxx"
#include "utility.hxx"


// Problem: If a parent process gets deleted, that has a child that waits for it, but the PID
// is reused before the scheduler checks the WAITING condition, the child stays in waiting condition!
// => Processes cannot easily be reused!
// Fix 1): To use increasing, unique ids. This would work and be the pragmatic solution, but fuck that
// Fix 2): Implement proper garbage collection using a background process that regulary checks the
//		   dependency chain and removes the processes.

namespace internal
{
	// A functor that converts a unique_ptr to a oberserver_ptr.
	// We sadly cant use lambdas here, since the process_manager class
	// needs to expose the type of the iterator as a type alias, so we
	// need to directly use the class "transform_iterator" that is
	// supplied by boost.
	template< typename T >
	struct convert_ptr
	{
		using to_type = ut::observer_ptr<T>;
	
		// Iterator of maps use pairs as value type
		template< typename U >
		auto operator()(const U& p_result) const
			-> to_type
		{
			return { p_result.second.get() };
		}
	};
}


class process_manager
	: public global_system
{
	static constexpr ::std::size_t pid_reserve_size = 32U;

	using process_ptr = ::std::unique_ptr<process>;
	using process_view = ut::observer_ptr<process>;
	using process_map = ::std::unordered_map<process_id, process_ptr>;
	using process_list = ::std::vector<process_view>;
	using pid_list = ut::small_vector<process_id, pid_reserve_size>;
	
	// Container type aliases
	using iterator = boost::transform_iterator<
						internal::convert_ptr<process>,
						process_map::iterator
					>;
					
	using const_iterator = boost::transform_iterator<
								internal::convert_ptr<const process>,
								process_map::const_iterator
							>;
							
	using size_type = process_map::size_type;
	using value_type = process_view;
	
	// Allow process base class methods to access housekeeping methods
	friend class process;
	
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
		auto initialize()
			-> void;
			
		auto frame()
			-> void;
			
		auto tick()
			-> void;	
		
	public:
		auto kill_process(process_id)	
			-> void;
				
		auto get_state(process_id) const
			-> process_state;
			
		auto get_process(process_id)
			-> process_view;
			
	public:
		auto begin()
			-> iterator;
			
		auto end()
			-> iterator;
			
		auto begin() const
			-> const_iterator;
			
		auto end() const
			-> const_iterator;
			
		auto cbegin() const
			-> const_iterator;
			
		auto cend() const
			-> const_iterator;
			
	private:
		// TODO really return view here? The view can be constructed
		// in create_process aswell.
		auto register_process(process_ptr)
			-> process_view;
	
		auto next_pid()
			-> process_id;
			
		auto free_pid(process_id)
			-> void;
		
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
		pid_list m_FreePids{ };			//< List for recycled PIDs
		process_id	m_NextPid{1U};		//< The next pid that may be used for a process
		process_map m_ProcMap;			//< Fast lookup map that owns all processes
		process_list m_PerFrameProcs;	//< Ordered (by priority) list of process views (per frame)
		process_list m_PerTickProcs;	//< Ordered (by priority) list of process views (per tick)
};
