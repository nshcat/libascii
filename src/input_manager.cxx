#include <GLXW/glxw.h>
#include <input_manager.hxx>
#include <global_state.hxx>

auto input_manager::initialize()
	-> void
{
	// Accessing the render_context is fine here, since it already has been initialized, according
	// to the initialization order.
	auto t_window = global_state<render_context>().handle();
	
	// Setup GLFW window to store a pointer to the input manager in the user data section.
	// This is not strictly needed, since the callback functions could just use global_state,
	// but this saves the calls and is a commonly used idiom.
	glfwSetWindowUserPointer(t_window, this);
}

auto input_manager::begin_input()
	-> void
{
	// Process all events that have been accumulated since last call of this method.
	glfwPollEvents();
}

auto input_manager::end_input()
	-> void
{
	// Clear key pressed that were only meant as one distinct press.
	// Keys that are in REPEAT state are not affected.
	this->keyDown.clear();
}

auto input_manager::process_key(key_type p_key, action_type p_action)
	-> void
{
	switch(p_action)
	{
		case GLFW_PRESS:
		{
			// Insert given key into the key down set.
			this->keyDown.insert(p_key);
			break;
		}
		case GLFW_REPEAT:
		{
			// Key is now considered as being repeatedly pressed, which means that it will
			// fire each frame. This state is only left when a button release event is
			// received.
			this->keyRepeated.insert(p_key);
			break;
		}
		case GLFW_RELEASE:
		default:
		{
			// Remove the key from both key down aswell as key repeated set,
			// since both states are canceled by this action.
			this->keyDown.erase(p_key);
			this->keyRepeated.erase(p_key);
			break;
		}
	}
}

auto input_manager::has_key(key_type p_key)
	-> bool
{
	return this->keyDown.count(p_key) || this->keyRepeated.count(p_key);
}

auto key_callback(GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods)
	-> void
{
	// Retrieve user pointer
	auto& t_mgr = *reinterpret_cast<input_manager*>(glfwGetWindowUserPointer(p_window));
	
	// Process key action
	if(p_key != GLFW_KEY_UNKNOWN)
		t_mgr.process_key(p_key, p_action);
}
