#pragma once

#include <boost/filesystem.hpp>
#include "global_system.hxx"

class path_manager
	: public global_system
{
	using path_type = boost::filesystem::path;

	public:
		auto initialize()
			-> void;
			
	public:
		// Returns the path to the configuration file. This depends on
		// the USE_HOME_DIR build setting.
		auto config_path() const
			-> const path_type&;
			
		// Returns the path to the games asset folder. This will always prioritize
		// asset folders present in the working directory.
		auto data_path() const
			-> const path_type&;
		
		// Path where the game can create user data, like save games.
		// This depends on the USE_HOME_DIR build setting.
		auto user_path() const
			-> const path_type&;

	private:
		// This will create the subfolder (roguelike) if it does not already exists
		auto init_user_path()
			-> void;
			
		auto init_data_path()
			-> void;
			
		auto init_config_path()
			-> void;
			
	public:
		auto working_directory() const
			-> path_type;
			
		auto home_directory() const
			-> path_type;
			
	private:
		path_type m_UserPath;
		path_type m_ConfigPath;
		path_type m_DataPath;
		
};
