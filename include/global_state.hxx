#pragma once

#include "render_context.hxx"

class global_state_t
{
	public:
		auto context()
			-> render_context&;
			
	private:
		render_context m_Context;
};

auto global_state()
	-> global_state_t&;
