#pragma once

#include <functional>
#include "edge_base.hxx"

namespace utility::pnfa::internal
{
	template< 	typename Tinput,
				typename... Tstate
	>
	class edge
		: public edge_base<Tinput, Tstate...>
	{
		protected:
			using this_type = edge<Tinput, Tstate...>;
			using base_type = edge_base<Tinput, Tstate...>;
			using action_fn = ::std::function<action_fn_t<Tinput, Tstate...>>;
			using cond_fn = ::std::function<condition_fn_t<Tinput, Tstate...>>;
	
		public:
			edge(cond_fn&& p_cond, action_fn&& p_action)
				: base_type(edge_type::normal, ::std::move(p_cond), ::std::move(p_action))
			{
			}
			
		public:
			virtual auto clone() const 
				-> ::std::unique_ptr<base_type>
			{
				return ::std::make_unique<this_type>(cond_fn{this->m_Cond}, action_fn{this->m_Action});
			}
			
	};
}
