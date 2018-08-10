#pragma once

#include "types.h"

extern "C"
{
	bool_t render_context_should_close();

	void render_context_begin_frame();

	void render_context_end_frame();
}
