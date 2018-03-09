#pragma once

#include <stdexcept>
#include <ut/observer_ptr.hxx>

#include "automaton_base.hxx"

namespace utility::pnfa
{
	template<	typename Tinput,
				typename... Tstate
	>
	class automaton;
	

	namespace internal
	{
		template<	typename Tinput,
					typename... Tstate
		>
		class sub_automaton
			: public node_base
		{
			using this_type = sub_automaton<Tinput, Tstate...>;
		
			protected:
				using node_id = node_base::node_id;
				using automaton_type = automaton<Tinput, Tstate...>;
				using base_type = automaton_base<Tinput, Tstate...>;
				using automaton_ptr = ::std::unique_ptr<base_type>;
				using automaton_view = ::ut::observer_ptr<automaton_type>;
			
			public:
				// We only accept the child automaton per rvalue, since that is how the
				// API will use this class. No other overloads are needed since this is just
				// an internal class that is not exposed to the user.
				sub_automaton(node_id, automaton_type&&);
			
			public:
				virtual auto clone() const
					-> ::std::unique_ptr<node_base>;
			
			public:
				auto view()
					-> automaton_view;
				
			protected:
				automaton_ptr m_Child{ };
		};
	}
}

