#pragma once

#include "edge_base.hxx"
#include "probability.hxx"
#include "utility.hxx"


namespace utility::pnfa::internal
{
	// An edge carrying a probability of being chosen
	template<	typename Tinput,
				typename... Tstate
	>	
	class probabilistic_edge
		: public edge_base<Tinput, Tstate...>
	{
		using base_type = edge_base<Tinput, Tstate...>;
		using action_fn = typename base_type::action_fn;
		using cond_fn = typename base_type::cond_fn;	
		
		public:
			probabilistic_edge(cond_fn&& p_cond, action_fn&& p_action, probability p_prob)
				: base_type(edge_type::probabilistic, ::std::move(p_cond), ::std::move(p_action)), m_Prob{p_prob}
			{
				this->m_Type = edge_type::probabilistic;
			}
			
		public:
			auto probability() const
				-> double
			{
				return m_Prob.value();
			}
	
		protected:
			class probability m_Prob;
	};
}
