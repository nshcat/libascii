#pragma once

#include "global_state.hxx"
#include "game_info.hxx"

class engine
{
	public:
		static auto initialize(const game_info&, int argc, const char** argv)
			-> void;
			
		static auto game_information()
			-> const game_info&;
			
	private:
		static game_info m_GameInfo;
};
