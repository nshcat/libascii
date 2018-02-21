#include <cl.hxx>


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
		cl::short_name('w'),
		cl::description("The width of the game window, in glyphs")
	}
};
