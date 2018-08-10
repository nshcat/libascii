#pragma once

#include <string>

#include <boost/filesystem.hpp>

// Class holding information set at build-time
class build_settings
{
	using path_type = boost::filesystem::path;

	public:
		// Prefix for data path. On linux, this could be something like "/usr/share".
		// The engine needs to append the game title to this to create the complete data
		// path.
		static auto data_path_prefix()
			-> path_type;
			
		// Whether the game should the users home directory
		// to store configuration and save data or not
		static auto use_home()
			-> bool;
			
		// Create string that contains all build settings with labels.
		// Used for debugging purposes.
		static auto dump_settings()
			-> ::std::string;
};
