#pragma once

namespace utility::pnfa
{
	// An enumeration detailing the different
	// states an automaton can be in
	enum class automaton_state
	{
		stopped,
		running,
		in_sub_automaton
	};
	
	enum class automaton_result
	{
		accepted,
		rejected,
		exit_from_sub,	//< Exited from sub automaton
		running		//< This result is returned if the automaton neither rejected nor accepted
					//  the input. This might happen when the automaton is executed step-by-step.
					//  It signals that the automaton expects more input values to determine
					//  the result.
	};
}
