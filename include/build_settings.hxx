#pragma once

#include <boost/filesystem.hpp>

// Class holding information set at build-time
class build_settings
{
	using path_type = boost::filesystem::path;

	public:
		// Path to game data (assets)
		static auto data_path()
			-> path_type;
			
		// Whether the game should the users home directory
		// to store configuration and save data or not
		static auto use_home()
			-> bool;
};
