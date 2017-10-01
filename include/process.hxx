#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <ut/bitmask.hxx>


using process_id = ::std::uint64_t;

// Special process id that means "no process"
constexpr static process_id no_process = process_id{0U};


// Enumeration of states a process can be in
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
				
	paused,		//< Process is paused and mustn't be run next time
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
			
	public:
		auto set_state(process_state)
			-> void;
			
		auto set_flags(process_flags)
			-> void;
			
		auto set_parent(process_id)
			-> void;
		
	protected:
		process_id m_Pid;		//< Unique process identificator
		process_id m_Parent{no_process};	//< PID of parent process
		process_state m_State{process_state::inactive}; //< Current state of the process
		process_flags m_Flags;	//< Additional process flags
};





