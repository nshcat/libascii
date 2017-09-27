#include <cstdlib>
#include <stdexcept>
#include <boost/filesystem/operations.hpp>
#include <ut/platform.hxx>

#if defined(LIBUT_IS_POSIX)
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <pwd.h>
#endif

#include <path_manager.hxx>
#include <build_settings.hxx>

// TODO MacOSX support

auto path_manager::home_directory() const
	-> path_type
{
	#if defined(LIBUT_IS_POSIX)
	// First try to retrieve the contents of the environment variable
	// "HOME"
	char* t_envVal = ::std::getenv("HOME");
	
	if(t_envVal)
		return { t_envVal };
		
	// If that failed, we need to use getpwuid
	struct passwd* t_pwd = getpwuid(getuid());
	
	if(t_pwd)
		return { t_pwd->pw_dir };
	else
		throw ::std::runtime_error("path_manager: Failed to obtain user home directory");
	
	#elif defined(LIBUT_IS_WINDOWS)
	// We want to save our player data in the directory stored
	// in the environment variable %APPDATA%.
	char* t_envVal = ::std::getenv("APPDATA");
	
	if(!t_envVal)
		throw ::std::runtime_error("path_manager: Failed to retrieve the contents of %APPDATA%");
	else return { t_envVal };
	#else
	#	error("Platform not supported")
	#endif
}

auto path_manager::working_directory() const
	-> path_type
{
	return boost::filesystem::current_path();
}

auto path_manager::init_user_path()
	-> void
{
	// Determine where to save user data
	if(build_settings::use_home())
	{
		// Use the home directory. We need to make sure that the subdirectory
		// exists (which is named after the game)
		auto t_path = home_directory();
		t_path /= ".roguelike";
		
		// Check if it exists
		if(!boost::filesystem::exists(t_path))
		{
			// Create it
			if(!boost::filesystem::create_directory(t_path))
				throw ::std::runtime_error("path_manager: Failed to create user data subdirectory");
		}
		else
		{
			// Make sure it is not present as a file
			if(!boost::filesystem::is_directory(t_path))
				throw ::std::runtime_error("path_manager: Target user data directory is a file");
		}
		
		m_UserPath = t_path;
	}
	else // Use the current working directory
	{
		m_UserPath = working_directory();
	}
}

auto path_manager::init_data_path()
	-> void
{
	// We need to check if there is an asset folder present in the
	// cwd, since we always prioritize that over the PREFIX build-time
	// setting.
	const auto t_localPath = working_directory() / "assets";
	
	if(boost::filesystem::exists(t_localPath)
		&& boost::filesystem::is_directory(t_localPath))
	{
		m_DataPath = t_localPath;
	}
	else // Use PREFIX build-time setting
	{
		const auto t_prefixPath = build_settings::data_path();
		
		if(boost::filesystem::exists(t_prefixPath)
			&& boost::filesystem::is_directory(t_prefixPath))
		{
			m_DataPath = t_prefixPath;
		}
		else
			throw ::std::runtime_error("path_manager: Could not determine data path");
	}
}

auto path_manager::init_config_path()
	-> void
{
	m_ConfigPath = user_path() / "config.json";
}

auto path_manager::initialize()
	-> void
{
	init_user_path();
	init_data_path();
	init_config_path();
}

auto path_manager::user_path() const
	-> const path_type&
{
	return m_UserPath;
}

auto path_manager::data_path() const
	-> const path_type&
{
	return m_DataPath;
}

auto path_manager::config_path() const
	-> const path_type&
{
	return m_ConfigPath;
}


