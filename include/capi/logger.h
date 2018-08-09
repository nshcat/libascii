#pragma once

#include "types.h"

extern "C"
{
	void logger_post_message(int lvl, const char* tag, const char* msg, bool_t bare);

	void logger_lock();
		
	void logger_unlock();
}
