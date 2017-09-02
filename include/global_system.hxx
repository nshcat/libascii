#pragma once

// Class that is used as base class for all global game systems.
// (purely because of semantic reasons)
class global_system
{
	public:
		global_system() = default;
		
		global_system(const global_system&) = delete;
		global_system(global_system&&) = default;
		
		global_system& operator=(const global_system&) = delete;
		global_system& operator=(global_system&&) = delete;
};

