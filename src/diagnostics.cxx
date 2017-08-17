#include <iostream>
#include <algorithm>
#include <type_traits>
#include <array>
#include <utility>
#include <cstdlib>
#include <iterator>
#include <cstring>
#include <diagnostics.hxx>


// Counters for errors and warnings
::std::size_t g_ErrorCount = 0;
::std::size_t g_WarningCount = 0;

// Whether debug output is enabled or not
bool g_Debug = false;

namespace internal
{
	bool debug_check(message_type p_type)
	{
		return p_type != message_type::debug || g_Debug;
	}

	void modify_counters(message_type p_type)
	{
		switch(p_type)
		{
			case message_type::error:
				++g_ErrorCount;
				break;
				
			case message_type::warning:
				++g_WarningCount;
				break;
				
			default:
				break;
		}
	}

	ut::string_view get_line(ut::string_view p_in, ::std::size_t p_line)
	{
			auto t_begin = p_in.begin();
			auto t_end = p_in.begin();

			for(;;++t_begin)
			{
				if(p_line <= 1)	// Reached line
					break;
			
				if(t_begin == p_in.end())
					throw ::std::runtime_error("get_line: Line number out of range");
						
				if(*t_begin == '\n')
					--p_line;	
			}

			// Find next \n
			t_end = ::std::find(t_begin, p_in.end(), '\n');

			const auto t_pos = ::std::distance(p_in.begin(), t_begin);
			const auto t_len = ::std::distance(t_begin, t_end);

			return p_in.substr(t_pos, t_len);
	}	
	
	ut::string_view get_type_str(message_type p_type)
	{
		const static ::std::array<const char*, 6> t_table = {{
			"error",
			"warning",
			"note",
			"info",
			"success",
			"debug"
		}};
		
		return { t_table.at(static_cast<::std::underlying_type_t<message_type>>(p_type)) };
	}
	
	
	ut::console_color get_type_clr(message_type p_type)
	{
		const static ::std::array<ut::console_color, 6> t_table = {{
			ut::console_color::bright_red,
			ut::console_color::bright_yellow,
			ut::console_color::bright_white,
			ut::console_color::bright_white,
			ut::console_color::bright_green,
			ut::console_color::bright_magenta		
		}};
		
		return t_table.at(static_cast<::std::underlying_type_t<message_type>>(p_type));
	}
}

void switch_debug(bool p_val)
{
	g_Debug = p_val;
}

void post_summary()
{		
	if(!g_ErrorCount && !g_WarningCount) return;

	if(g_ErrorCount && g_WarningCount)
	{
		::std::cout 	<< '\n' << g_ErrorCount << " error"
						<< (g_ErrorCount == 1 ? "": "s")
						<< " and " << g_WarningCount
						<< " warning" 
						<< (g_WarningCount == 1 ? "": "s")
						<< " generated." << std::endl;
		return;
	}
	else if(g_ErrorCount)
	{
		::std::cout 	<< '\n' << g_ErrorCount << " error"
						<< (g_ErrorCount == 1 ? "": "s")
						<< " generated." << std::endl;
		return;
	}
	else
	{
		::std::cout 	<< '\n' << g_WarningCount << " warning"
						<< (g_WarningCount == 1 ? "": "s")
						<< " generated." << std::endl;
		return;
	}
}

void post_empty()
{
	::std::cout << ::std::endl;
}

void post_diagnostic(message_type p_type, ut::string_view p_sender, ut::string_view p_message)
{
	if(!internal::debug_check(p_type))
		return;

	const auto t_color = internal::get_type_clr(p_type);
	const auto t_type_str = internal::get_type_str(p_type);
	
	::std::cout << p_sender << ": "
				<< ut::foreground(t_color)
				<< t_type_str
				<< ut::reset_color
				<< ": "
				<< p_message
				<< ::std::endl;
}

void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, ut::string_view p_message)
{
	if(!internal::debug_check(p_type))
		return;

	const auto t_color = internal::get_type_clr(p_type);
	const auto t_type_str = internal::get_type_str(p_type);
	
	internal::modify_counters(p_type);
	
	::std::cout << p_sender << ":"
				<< p_pos.first << ":"
				<< p_pos.second << ": "
				<< ut::foreground(t_color)
				<< t_type_str
				<< ut::reset_color
				<< ": "
				<< p_message
				<< ::std::endl;
}

void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, caret p_caret, ut::string_view p_src, ut::string_view p_message)
{
	if(!internal::debug_check(p_type))
		return;
	
	const auto t_line = internal::get_line(p_src, p_pos.first);

	post_diagnostic(p_type, p_sender, p_pos, p_message);
	::std::cout << "\t" << t_line << "\n\t";
	
	::std::cout << foreground(ut::console_color::bright_green);
	
	// Fill with blanks from start to caret position
	::std::fill_n(::std::ostream_iterator<char>(::std::cout), p_caret.first, ' ');
	::std::cout << '^';
	
	// Add squiggly lines if needed
	if(p_pos.second > 1)
		::std::fill_n(::std::ostream_iterator<char>(::std::cout), p_caret.second-1, '~');
		
	::std::cout << ut::reset_color << '\n';	
}

void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, caret p_caret, ut::string_view p_src, ut::string_view p_fix, ut::string_view p_message)
{
	if(!internal::debug_check(p_type))
		return;

	post_diagnostic(p_type, p_sender, p_pos, p_caret, p_src, p_message);
	
	::std::cout << ut::foreground(ut::console_color::bright_green) << '\t';
	::std::fill_n(::std::ostream_iterator<char>(::std::cout), p_caret.first, ' ');
	::std::cout << p_fix << '\n' << ut::reset_color;
}

void post_ice(ut::string_view p_sender, ut::string_view p_message, bool p_abort)
{
	const auto t_str = ::std::string("internal compiler error:\n\t") + p_message.to_string();
	post_diagnostic(message_type::error, p_sender, {t_str});
	
	if(p_abort)
		::std::exit(EXIT_FAILURE);
}

void post_ice(ut::string_view p_sender, const ::std::exception& p_ex, bool p_abort)
{
	const auto t_str = ::std::string("An exception was thrown. Output of what():\n\n\t") + p_ex.what();		
	post_ice(p_sender, {t_str}, p_abort);
}

