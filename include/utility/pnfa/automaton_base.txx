namespace utility::pnfa
{
	namespace internal
	{
		template<typename... Tstate>
		automaton_base<no_input, Tstate...>::~automaton_base()
		{
		}
		
		template<typename Tinput, typename... Tstate>
		automaton_base<Tinput, Tstate...>::~automaton_base()
		{
		}	

		template< typename... Tstate >
		auto automaton_base<no_input, Tstate...>::step(Tstate&... p_states)
			-> automaton_result
		{
			auto t_ptr = dynamic_cast<automaton<no_input, Tstate...>*>(this);
		
			return t_ptr->step_impl(no_input{ }, p_states...);
		}
		
		template< typename Tinput, typename... Tstate >
		auto automaton_base<Tinput, Tstate...>::step(const Tinput& p_in, Tstate&... p_states)
			-> automaton_result
		{
			auto t_ptr = dynamic_cast<automaton<Tinput, Tstate...>*>(this);
		
			return t_ptr->step_impl(p_in, p_states...);
		}
	}
}
