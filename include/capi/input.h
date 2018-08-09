#pragma once

#include "types.h"

extern "C"
{
	void input_begin();

	void input_end();

	bool_t input_has_key(int key);
}
