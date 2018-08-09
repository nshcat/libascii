#include <cstring>
#include <cstdlib>
#include <capi/memory.h>

extern "C"
{
	void free_memory(void* p_ptr)
	{
		::std::free(p_ptr);
	}
}

