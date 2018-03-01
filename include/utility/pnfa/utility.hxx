#pragma once

#include <ut/type_traits.hxx>
#include <type_traits>

namespace utility::pnfa
{
	// Special input type used to tell the automaton that no input
	// should be processed. Using this will change the expected
	// signature for the condition functions attached to edges
	// to not expect an input symbol.
	struct no_input{ };

	namespace internal
	{
		// Determine signature of the condition function used
		// to determine whether an edge can be traversed.
		// If the given input type is `no_input`, no input symbol
		// is expected as first argument.
		template< typename Tinput, typename... Tstates >
		using condition_fn_t = ::std::conditional_t<
									::std::is_same_v<Tinput, no_input>,
									bool(const Tstates&...),
									bool(const Tinput&, const Tstates&...)
							   >;
							   
		// Determine signature of action function
		template< typename Tinput, typename... Tstates >
		using action_fn_t = ::std::conditional_t<
									::std::is_same_v<Tinput, no_input>,
									void(Tstates&...),
									void(const Tinput&, Tstates&...)
							   >;
	}
}
