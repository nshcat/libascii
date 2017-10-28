#pragma once

#include <type_traits>
#include <ut/type_traits.hxx>

// Class that is used as base class for all global game systems.
// (purely because of semantic reasons)
//
// A global system is required to provide the following methods:
//  - `auto initialize() -> void`
//  - `auto shutdown() -> void`
//
// TODO: try to enforce this, maybe using CRTP (and then `static_assert`-ing
//		 on the given type here)
class global_system
{
	public:
		global_system() = default;
		
		global_system(const global_system&) = delete;
		global_system(global_system&&) = default;
		
		global_system& operator=(const global_system&) = delete;
		global_system& operator=(global_system&&) = delete;
};


// A trait that detects a member function called "initialize"
template< typename T >
using detect_initialize = decltype(::std::declval<T>().initialize());

template< typename T >
using has_initialize = ut::is_detected<detect_initialize, T>;

template< typename T >
constexpr bool has_initialize_v = has_initialize<T>::value;
//

// A trait that detects a member function called "shutdown"
template< typename T >
using detect_shutdown = decltype(::std::declval<T>().shutdown());

template< typename T >
using has_shutdown = ut::is_detected<detect_shutdown, T>;

template< typename T >
constexpr bool has_shutdown_v = has_shutdown<T>::value;
//

// A trait that determines whether a type can be classified as 
// a "global system"
template< typename T >
using is_global_system = ::std::conjunction<
							has_shutdown<T>,
							has_initialize<T>,
							::std::is_trivially_default_constructible<T>,
							::std::is_trivially_destructible<T>
						>;

template< typename T >
constexpr bool is_global_system_v = is_global_system<T>::value;
//
