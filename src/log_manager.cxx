#include <log_manager.hxx>
#include <commandline.hxx>
#include <global_state.hxx>
#include <path_manager.hxx>

auto log_manager::initialize()
	-> void
{
	// Initialize the logger in empty state
	lg::logger::null_init();
	
	// Retrieve console severity level threshold
	auto t_level = g_clHandler.value<lg::severity_level>(cl_argument::logger_verbosity);

	// If verbose mode is activated, set level to debug
	if(g_clHandler.value<bool>(cl_argument::logger_verbose))
		t_level = lg::severity_level::debug;

	// Create and add console target
	m_Console = ::std::make_unique<console_target>(t_level);
	lg::logger::add_target(m_Console.get());
	
	// Add file target, if requested
	if(g_clHandler.value<bool>(cl_argument::logger_enable_file))
	{
		// Build path. This depends on the USE_HOME_DIR build setting.
		// Note that the path manager is initialized before the log manager,
		// so accessing it is fine here.
		const auto t_path = global_state<path_manager>().user_path() / "app.log";
	
		// Retrieve flag telling us whether to truncate the log file or not
		const auto t_append = g_clHandler.value<bool>(cl_argument::logger_append_file);
	
		// Always use debug severity level when outputting to the log file
		m_File = ::std::make_unique<file_target>(lg::severity_level::debug, t_path.string(), t_append);
		lg::logger::add_target(m_File.get());
	}
}

auto log_manager::shutdown()
	-> void
{
	lg::logger::shutdown();
}
