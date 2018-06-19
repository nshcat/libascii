#pragma once

#include "global_system.hxx"
#include "global_state_impl.hxx"

#include "render_context.hxx"
#include "renderer.hxx"
#include "lighting.hxx"
#include "path_manager.hxx"
#include "configuration.hxx"
#include "asset_manager.hxx"
#include "palette_loader.hxx"
#include "process_manager.hxx"
#include "log_manager.hxx"
#include "input_manager.hxx"

// Define global state type by providing initialization
// sequence of the subsystems. They will be deinitialized
// in reverse order.
using global_state_t = internal::global_state_impl<
	path_manager,
	log_manager,
	configuration,
	process_manager,
	render_context,
	input_manager,
	asset_manager,
	render_manager,
	light_manager
>;


auto global_state()
	-> global_state_t&;
	
	
template< typename T >
auto global_state()
	-> T&
{
	// TODO assert that T is actually included in the tuple of systems to
	// give good error message here, and not later on in std::get
	
	static_assert(is_global_system_v<T>,
		"T needs to be a global system!");
		
	return ::std::get<T>(global_state().systems());	
}
