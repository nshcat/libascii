#pragma once

#include <memory>
#include <utility>
#include <unordered_map>
#include <type_traits>
#include <ut/observer_ptr.hxx>

#include "global_system.hxx"
#include "process.hxx"


class process_manager
	: public global_system
{
	using process_ptr = ::std::unique_ptr<process>;
	using process_view = ut::observer_ptr<process>;
	using process_map = ::std::unordered_map<process_id, process_ptr>;
	
	public:
		auto initialize()
			-> void;
			
		auto frame()
			-> void;
			
		auto tick()
			-> void;
		
	public:
		template< typename T >
		auto create_process(
		
	public:
		auto kill_process(process_id)	
			-> void;
			
		auto pause_process(process_id, ::std::size_t)
			-> void;
			
		auto wait_for_parent(process_id)
			-> void;
		
		auto process_state(process_id)
			-> class process_state;
			
		auto process(process_id);
			-> process_view;
			
	private:
		
};
