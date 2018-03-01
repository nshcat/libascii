#pragma once

#include "enum.hxx"
#include "utility.hxx"

namespace utility::pnfa
{
	namespace internal
	{
		// The base for the automaton class. It is used to mix-in API methods
		// that differ in their signature based on whether Tinput is equals to
		// `no_input` or not.
		// This is the base case, where the input type is not `no_input`.
		template<	typename Tinput,
					typename... Tstate
		>
		class automaton_base
		{
			public:
				// Disallow construction of this class
				virtual ~automaton_base() = 0;
		
			public:
				auto step(const Tinput&, Tstate&...)
					-> automaton_result;
		};
		
		// Special case where the input type is `no_input`, which means that
		// no input values are expected.
		template< typename... Tstate >
		class automaton_base<no_input, Tstate...>
		{		
			public:
				// Disallow construction of this class
				virtual ~automaton_base() = 0;
				
			public:
				auto step(Tstate&...)
					-> automaton_result;
		};
	}
}
