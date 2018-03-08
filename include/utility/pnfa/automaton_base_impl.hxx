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
		
		template< typename Tinput, typename... Tstate >
		auto automaton_base<Tinput, Tstate...>::run(ut::array_view<const Tinput> p_in, Tstate&... p_states)
			-> automaton_result
		{
			auto t_ptr = dynamic_cast<automaton<Tinput, Tstate...>*>(this);
		
			automaton_result t_res{ automaton_result::running };
		
			// Go through whole input sequence and perform steps
			for(const auto& t_in: p_in)
			{
				t_res = this->step(t_in, p_states...);
				
				// Check return value. If it is accepted, we don't care yet since there might
				// be input left. If it is rejected though, there was no edge that could be taken,
				// so we reject the whole input sequence.
				if(t_res == automaton_result::rejected)
					break;	//< We can't just return here, since the automaton needs to be reset.
			}
			
			// We consumed the whole input sequence, and there was always an edge
			// that could be taken. If the last step ended on an accepting node,
			// the `step` call returned `accepted`, and so do we.
			if(t_res != automaton_result::accepted)
				t_res = automaton_result::rejected;	//< We also reject if result is `running`, or `exit_sub` since
													//  we didn't reach an accepting node with the input
													//	we got.												
					
			// Since `run` gets the whole input sequence, we can actually reset the automaton
			// now.
			t_ptr->reset();							
						
			// Return result indicator
			return t_res;
		}
	}
}
