#include <capi/path.h>
#include <memory>
#include <algorithm>
#include <cstring>
#include <global_state.hxx>

extern "C"
{
	const char* path_get_data_path()
	{
		const auto p = global_state<path_manager>().data_path().string();
		char* str = new char[p.size()+1];
		::std::copy(p.begin(), p.end(), str);
		str[p.size()] = '\0';
		
		return str;
	}
	
	const char* path_get_user_path()
	{
		const auto p = global_state<path_manager>().user_path().string();
		char* str = new char[p.size()+1];
		::std::copy(p.begin(), p.end(), str);
		str[p.size()] = '\0';
		
		return str;
	}	
}

