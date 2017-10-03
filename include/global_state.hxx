#pragma once

#include "render_context.hxx"
#include "path_manager.hxx"
#include "configuration.hxx"
#include "asset_manager.hxx"
#include "palette_loader.hxx"
#include "process_manager.hxx"

class global_state_t
{
	public:
		auto context()
			-> render_context&;
			
		auto path_manager()
			-> class path_manager&;
			
		auto process_manager()
			-> class process_manager&;
			
		auto asset_manager()
			-> class asset_manager&;
			
		auto configuration()
			-> class configuration&;
			
	public:
		auto initialize()
			-> void;
			
	private:
		render_context m_Context;
		class path_manager m_PathManager;
		class process_manager m_ProcessManager;
		class asset_manager m_AssetManager;
		class configuration m_Configuration;
};

auto global_state()
	-> global_state_t&;
