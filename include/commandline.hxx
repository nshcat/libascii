#pragma once

#include <cl.hxx>

enum class commandline
{
	logger_verbosity,
	logger_verbose,
	logger_enable_file,
	logger_append_file
};

extern cl::handler g_clHandler;
