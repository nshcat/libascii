#include <capi/config.h>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <log.hxx>
#include <ut/string_view.hxx>
#include <global_state.hxx>



namespace internal
{
	template< typename T >
	T retrieve_config_value(ut::string_view p_path)
	{
		const auto t_val = global_state<configuration>().get<T>({p_path.to_string()});
		
		if(!t_val)
		{
			LOG_F_TAG("libascii") << "Requested configuration entry does not exist or type mismatch: \"" << (p_path) << "\"";
			throw new ::std::runtime_error("requested configuration entry does not exist or type mismatch");
		}
		
		return *t_val;
	}
}

extern "C"
{
	float configuration_get_float(const char* p_path)
	{
		return internal::retrieve_config_value<float>({p_path});
	}
	
	double configuration_get_double(const char* p_path)
	{
		return internal::retrieve_config_value<double>({p_path});
	}
	
	int configuration_get_int(const char* p_path)
	{
		return internal::retrieve_config_value<int>({p_path});
	}
	
	unsigned configuration_get_uint(const char* p_path)
	{
		return internal::retrieve_config_value<unsigned>({p_path});
	}
	
	const char* configuration_get_string(const char* p_path)
	{
		const auto t_str = internal::retrieve_config_value<::std::string>({p_path});
		
		::std::unique_ptr<char[]> t_buf{ new char[t_str.length() + 1] };
		
		::std::copy(t_str.cbegin(), t_str.end(), t_buf.get());
		t_buf[t_str.length()] = '\0';
		
		return t_buf.release();
	}
	
	bool_t configuration_get_boolean(const char* p_path)
	{
		return static_cast<bool_t>(internal::retrieve_config_value<bool>({p_path}));
	}
}
