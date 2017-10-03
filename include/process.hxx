#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <ut/bitmask.hxx>

// TODO process title and description


using process_id = ::std::uint64_t;

// Special process id that means "no process"
constexpr static process_id no_process = process_id{0U};

constexpr static ::std::size_t no_sleep = ::std::size_t{0U};


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
	none = 0U	//< No flags
};

LIBUT_MAKE_BITMASK(process_flags)
//


class process
{
	public:
		process(process_id p_id, process_id p_parent, process_type p_type);
		
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
			
	public:
		auto set_wait_pid(process_id)
			-> void;
	
		auto set_state(process_state)
			-> void;
			
		auto set_flags(process_flags)
			-> void;
			
		auto set_parent(process_id)
			-> void;
			
		auto set_sleep_duration(::std::size_t)
			-> void;
			
		auto dec_sleep_duration()
			-> void;
			
	public:
		auto sleep(::std::size_t)
			-> void;
			
		auto kill()
			-> void;
			
		auto wait_for(process_id)
			-> void;
		
	protected:
		process_id m_Pid;								//< Unique process identificator
		process_id m_Parent{no_process};				//< PID of parent process
		process_id m_WaitPid{no_process};				//< Process this process is waiting for
		process_type m_Type{process_type::per_frame};	//< Type of this process
		process_state m_State{process_state::inactive}; //< Current state of the process
		process_flags m_Flags{process_flags::none};		//< Additional process flags
		::std::size_t m_SleepDuration{no_sleep};		//< Duration the process still has to sleep
};





