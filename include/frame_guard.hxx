#pragma once

#include <exception>
#include "global_state.hxx"

// RAII-style frame begin/end manager
class frame_guard
{
	public:
		frame_guard()
			: 	m_ExcCount{ ::std::uncaught_exceptions() },
				m_Context{ global_state().context() }
		{
			m_Context.begin_frame();
		}
	
		~frame_guard()
		{
			// Only end frame if we are not currently unwinding
			// the stack because of an exception being thrown
			// from within the render code
			if(m_ExcCount >= ::std::uncaught_exceptions())
				m_Context.end_frame();
		}
	
	private:
		int m_ExcCount;				//< Number of exceptions active when this object
									//  was constructed
		render_context& m_Context;	//< We cache the reference to the context manager
									//  since its address will never change
};
