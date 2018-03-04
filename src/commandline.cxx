#include <log.hxx>
#include <commandline.hxx>

using namespace std::literals::string_literals;


cl::handler g_clHandler{
	// Application info
	cl::application_name("roguelike"),

	// Enable help display
	cl::help_argument{ },
	
	// Window width in glyphs
	cl::integer_argument<int>{
		cl::default_value(80),
		cl::max(255),
		cl::min(50),
		cl::long_name("width"),
		cl::short_name('W'),
		cl::description("The width of the game window, in glyphs"),
		cl::category("Game settings")
	},
	
	cl::integer_argument<int>{
		cl::default_value(80),
		cl::max(255),
		cl::min(50),
		cl::long_name("height"),
		cl::short_name('H'),
		cl::description("The height of the game window, in glyphs"),
		cl::category("Game settings")
	},
	
	cl::string_argument{
		cl::default_value("default"s),
		cl::long_name("tileset"),
		cl::description("Sets the tileset used by the game"),
		cl::category("Game settings")
	},
	
	cl::enum_argument<lg::severity_level>{
		cl::id(commandline::logger_verbosity),
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
		cl::id(commandline::logger_verbose),
		cl::long_name("verbose"),
		cl::category("Logger"),
		cl::short_name('v'),
		cl::default_value(false),
		cl::description("Causes the logger to output all message to the console, regardless of their severity")
	},
	
	cl::boolean_argument
	{
		cl::id(commandline::logger_append_file),
		cl::long_name("append-log"),
		cl::category("Logger"),
		cl::short_name('A'),
		cl::default_value(false),
		cl::description("Causes the logger to append to log file instead of truncating it")
	},
	
	cl::boolean_argument
	{
		cl::id(commandline::logger_enable_file),
		cl::long_name("log-to-file"),
		cl::category("Logger"),
		cl::short_name('F'),
		cl::default_value(false),
		cl::description("Determines if the logger also writes its output to a file")
	}
};
