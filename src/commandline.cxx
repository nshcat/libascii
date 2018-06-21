#include <log.hxx>
#include <commandline.hxx>
#include <global_state.hxx>

using namespace std::literals::string_literals;


cl::handler g_clHandler{
	// Application info
	cl::application_name("roguelike"),

	// Enable help display
	cl::help_argument{ },
	
	cl::enum_argument<lg::severity_level>{
		cl::id(cl_argument::logger_verbosity),
		cl::long_name("verbosity"),
		cl::category("Logger"),
		cl::short_name('V'),
		cl::description("Sets the threshold for logging messages to be shown on the console"),
		cl::default_value(lg::severity_level::info),
		cl::ignore_case,
		
		cl::enum_key_value("all", 		lg::severity_level::debug),
		cl::enum_key_value("debug", 	lg::severity_level::debug),
		cl::enum_key_value("info", 		lg::severity_level::info),
		cl::enum_key_value("warning", 	lg::severity_level::warning),
		cl::enum_key_value("error", 	lg::severity_level::error),
		cl::enum_key_value("fatal", 	lg::severity_level::fatal)
	},
	
	cl::boolean_argument
	{
		cl::id(cl_argument::logger_verbose),
		cl::long_name("verbose"),
		cl::category("Logger"),
		cl::short_name('v'),
		cl::default_value(false),
		cl::description("Causes the logger to output all message to the console, regardless of their severity")
	},
	
	cl::boolean_argument
	{
		cl::id(cl_argument::logger_append_file),
		cl::long_name("append-log"),
		cl::category("Logger"),
		cl::short_name('A'),
		cl::default_value(false),
		cl::description("Causes the logger to append to log file instead of truncating it")
	},
	
	cl::boolean_argument
	{
		cl::id(cl_argument::logger_enable_file),
		cl::long_name("log-to-file"),
		cl::category("Logger"),
		cl::short_name('F'),
		cl::default_value(false),
		cl::description("Determines if the logger also writes its output to a file")
	}
};


::std::vector<::std::string> g_argv;


auto populate_argv(int argc, const char** argv)
	-> void
{
	for(int i = 0; i < argc; ++i)
	{
		g_argv.push_back(::std::string{argv[i]});
	}
}

auto commandline::initialize()
	-> void
{
	// Our only job here is to parse the commandline arguments stored in the global
	// variables in this translation unit.
	::std::vector<const char*> t_ptrs{ };
	for(int i = 0; i < g_argv.size(); ++i)
		t_ptrs.push_back(g_argv[i].c_str());
	
	g_clHandler.read({ t_ptrs.begin(), t_ptrs.end() });
	
	// Read overrides
	global_state<configuration>().populate_overrides();
}
