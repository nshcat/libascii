#pragma once

typedef struct
{
	const char* name;
	const char* version;
	const char* description;
	const char* windowTitle;
} game_info_t;

extern "C"
{
	void engine_initialize(game_info_t* info, int argc, const char** argv);
}
