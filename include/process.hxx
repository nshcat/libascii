#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <limits>

#include <ut/bitmask.hxx>

// TODO process title and description


using process_id = ::std::uint64_t;

// Special process id that means "no process"
constexpr static process_id no_process = process_id{0U};

constexpr static ::std::size_t no_sleep = ::std::size_t{0U};
constexpr static ::std::size_t no_limit = ::std::size_t{0U};


// Enumeration of states a process can be in
// TODO: make a special state that is for processes that are currently receiving
// their time slice
enum class process_state
	: ::std::uint8_t
{
	nonexistent, //< Requested process does not exists.

	inactive,	//< Process is not yet initialized.

	active,		//< Process is active and ready to be run for the next
				//  time slice
		
	waiting,	//< Process is initialized, but its preconditions for
				//  for running are not yet met. (e.g. parent not
				//  finished yet)
				
	sleeping,	//< Process is sleeping and mustn't be run next time
				//  slice

	dead		//< Process is finished or was killed
};
//


// Supported types of processes
enum class process_type
	: ::std::uint8_t
{
	per_frame,	//< Process is given execution time each frame
	
	per_tick	//< Process is given execution time each game
				//  simulation tick
};
//


// Additional flags processes can have
enum class process_flags
	: ::std::uint32_t
{
	none = 0U,						//< No flags
	
	periodic_sleep = 1U,			//< Causes the process to periodically sleep. This means after
									//  every time slice, the process will sleep for set amount of time
									//	units.
	
	limited_runtime = 1U << 1,		//< Process will be killed after a certain number
									//  of allotted time slices.
									//  TODO other name? "kill_after", "auto_kill"
};

LIBUT_MAKE_BITMASK(process_flags)
//


// Process priority. Smaller is higher priority.
enum class process_priority
	: ::std::uint32_t
{
	highest = 0U,
	lowest = ::std::numeric_limits<::std::uint32_t>::max(),
	normal = lowest / 2U
};
//


class process
{
	public:
		process(process_id p_id, process_id p_parent, process_type p_type, process_priority p_prio);
		
	public:
		// Copying a process does not make any sense
		process(const process&) = delete;
		process& operator=(const process&) = delete;	
		
	public:
		// Is called once to setup the process
		virtual auto initialize() -> void = 0;
		
		// Is called for every assigned time slice
		virtual auto update() -> void = 0;
		
	public:
		auto pid() const
			-> process_id;
			
		auto parent() const
			-> process_id;
			
		auto state() const
			-> process_state;
		
		auto flags() const
			-> process_flags;
			
		auto type() const
			-> process_type;
			
		auto sleep_duration() const
			-> ::std::size_t;
			
		auto wait_pid() const
			-> process_id;
			
		auto priority() const
			-> process_priority;
			
		auto runtime() const
			-> ::std::size_t;
			
		// Amount of time slices this process can
		// run for before being killed.
		auto runtime_limit() const
			-> ::std::size_t;
			
		auto periodic_duration() const
			-> ::std::size_t;
			
	public:
		auto set_wait_pid(process_id)
			-> void;
	
		auto set_state(process_state)
			-> void;
			
		auto set_flags(process_flags)
			-> void;
			
		auto set_parent(process_id)
			-> void;
		
		auto set_periodic_duration(::std::size_t)
			-> void;
			
		auto set_sleep_duration(::std::size_t)
			-> void;
			
		auto dec_sleep_duration()
			-> void;
			
		auto set_runtime_limit(::std::size_t)
			-> void;
		
		auto inc_runtime()
			-> void;
			
	public:
		// Put process to sleep for given amount of time slices.
		// If the process is already sleeping, this will do nothing
		// (It will NOT update the sleep duration)
		auto sleep(::std::size_t)
			-> void;
			
		// After each assigned time slice, automatically put process to 
		// sleep for the fiven amount of time slices. Like `sleep`, this will
		// not initially update the sleep duration if the process is already sleeping.
		// If the flag is set, the process will start with sleeping instead of being active.
		auto periodic_sleep(::std::size_t p_duration, bool p_initialSleep = false)
			-> void;
			
		auto kill()
			-> void;
			
		auto wait_for(process_id)
			-> void;
			
		// TODO: better name?
		auto kill_after(::std::size_t)
			-> void;
		
	protected:
		process_id m_Pid;										//< Unique process identificator
		process_id m_Parent{no_process};						//< PID of parent process
		process_id m_WaitPid{no_process};						//< Process this process is waiting for
		process_priority m_Priority{process_priority::normal};	//< Priority of this process
		process_type m_Type{process_type::per_frame};			//< Type of this process
		process_state m_State{process_state::inactive}; 		//< Current state of the process
		process_flags m_Flags{process_flags::none};				//< Additional process flags
		::std::size_t m_SleepDuration{no_sleep};				//< Duration the process still has to sleep
		::std::size_t m_Runtime{};								//< Current process runtime duration
		::std::size_t m_RuntimeLimit{no_limit};					//< Process runtime limitation used by auto kill.
		::std::size_t m_PeriodicDuration{no_sleep};				//< Duration used to reset the sleep duration with periodic_sleep.
};





