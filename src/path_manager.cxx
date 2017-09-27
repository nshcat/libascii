#include <cstdlib>
#include <stdexcept>

#include <ut/platform.hxx>

#if defined(LIBUT_IS_POSIX)
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <pwd.h>
#endif

#include <path_manager.hxx>

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
