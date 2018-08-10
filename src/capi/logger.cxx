#include <capi/logger.h>
#include <cstring>
#include <log.hxx>
#include <ut/cast.hxx>
#include <ut/console_color.hxx>

namespace internal
{
	auto level_color(lg::severity_level p_lvl)
		-> ut::console_color
	{
		switch(p_lvl)
		{
			case lg::severity_level::fatal:
				return ut::console_color::bright_red;
				break;
			case lg::severity_level::error:
				return ut::console_color::bright_red;
				break;
			case lg::severity_level::warning:
				return ut::console_color::bright_yellow;
				break;
			case lg::severity_level::info:
				return ut::console_color::bright_white;
				break;	
			case lg::severity_level::debug:
				return ut::console_color::bright_cyan;
				break;
			default:
				LOG_F_TAG("libascii") << "Invalid log message severity supplied: " << ut::enum_cast(p_lvl);
				::std::exit(EXIT_FAILURE);	
		}
	}
}

extern "C"
{
	void logger_post_message(int p_lvl, const char* p_tag, const char* p_msg, bool_t p_bare)
	{
		const auto t_bare = static_cast<bool>(p_bare);
		const auto t_lvl = ut::enum_cast<lg::severity_level>(p_lvl);
		const auto t_clr = internal::level_color(t_lvl);	
		
		if(p_tag == nullptr || ::std::strlen(p_tag) == 0)
			LOGGER() += lg::log_entry("libascii", 0, t_bare) << t_lvl << t_clr << p_msg;
		else
			LOGGER() += lg::log_entry("libascii", 0, t_bare) << t_lvl << t_clr << lg::tag(::std::string{p_tag}) << p_msg;
	}
	
	void logger_lock()
	{
		LOG_LOCK();
	}
	
	void logger_unlock()
	{
		LOG_UNLOCK();
	}
}
