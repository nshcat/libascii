#pragma once

#include "render_context.hxx"
#include "path_manager.hxx"
#include "configuration.hxx"

class global_state_t
{
	public:
		auto context()
			-> render_context&;
			
		auto path_manager()
			-> class path_manager&;
			
		auto configuration()
			-> class configuration&;
			
	public:
		auto initialize()
			-> void;
			
	private:
		render_context m_Context;
		class path_manager m_PathManager;
		class configuration m_Configuration;
};

auto global_state()
	-> global_state_t&;
