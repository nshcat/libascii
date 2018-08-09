#pragma once

#include <cstdint>

#include "types.h"

typedef enum
{
	CMD_SET_GLYPH,
	CMD_SET_BG,
	CMD_SET_FG,
	CMD_SET_DEPTH,
	CMD_CLEAR_TILE,
	CMD_SET_GUI_MODE,
	CMD_SET_LIGHT_MODE
} command_type_t;

typedef struct
{
	command_type_t type;
	uvec2_t position;
	
	union
	{
		uint32_t value;
		uvec3_t color;
	};
} command_t;

extern "C"
{
	void screen_get_dimensions(uvec2_t* out);

	void screen_set_tile(uvec2_t* pos, uvec3_t* front, uvec3_t* back, uint8_t glyph);

	void screen_clear_tile(uvec2_t* pos);

	void screen_apply_commands(command_t* p_cmdbuf, int p_count);

	void screen_set_light_mode(uvec2_t* pos, int mode);

	void screen_set_gui_mode(uvec2_t* pos, bool_t flag);

	void screen_clear();

	void screen_set_depth(uvec2_t* pos, uint8_t depth);	
}
