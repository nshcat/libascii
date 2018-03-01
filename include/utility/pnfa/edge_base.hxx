#pragma once

#include "utility.hxx"


namespace utility::pnfa::internal
{
	enum class edge_type
	{
		normal,
		probabilistic
	};
	

	template< 	typename Tinput,
				typename... Tstate
	>
	class edge_base
	{
		protected:
			using action_fn = ::std::function<action_fn_t<Tinput, Tstate...>>;
			using cond_fn = ::std::function<condition_fn_t<Tinput, Tstate...>>;
	
		public:
			edge_base(edge_type p_type, cond_fn&& p_cond, action_fn&& p_action)
				: m_Type{p_type}, m_Cond{p_cond}, m_Action{p_action}
			{
			}
			
			virtual ~edge_base() = 0;
			
		public:
			auto is_probabilistic() const
				-> bool
			{
				return type() == edge_type::probabilistic;
			}
			
			auto type() const
				-> edge_type
			{
				return m_Type;
			}
			
			auto condition()
				-> cond_fn&
			{
				return m_Cond;
			}
			
			auto action()
				-> action_fn&
			{
				return m_Action;
			}
	
		protected:
			edge_type m_Type;
			cond_fn m_Cond;
			action_fn m_Action;
	};
	
	template< 	typename Tinput,
				typename... Tstate
	>
	edge_base<Tinput, Tstate...>::~edge_base()
	{
	}
}
