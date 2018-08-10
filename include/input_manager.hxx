#pragma once

#include <unordered_set>

#include <GLFW/glfw3.h>
#include "global_system.hxx"

class input_manager
	: public global_system
{
	// GLFW uses int to represent a key.
	using key_type = int;
	using action_type = int;

	public:
		auto initialize()
			-> void;
			
	public:
		// This should be called at the very beginning of every frame.
		auto begin_input()
			-> void;
	
		// This should be called at the end of every frame.
		auto end_input()
			-> void;
			
		// Query whether given key is pressed or not.
		// This uses the symbolic key name provided by GLFW.
		auto has_key(key_type p_key)
			-> bool;
			
	public:
		auto process_key(key_type p_key, action_type p_action)
			-> void;
			
	private:
		::std::unordered_set<key_type> keyDown;
		::std::unordered_set<key_type> keyRepeated;
};

// Callback functions that will be called by GLFW on key events
auto key_callback(GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods)
	-> void;
