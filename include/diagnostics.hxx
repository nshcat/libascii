#pragma once

#include <utility>
#include <exception>
#include <iostream>

#include <ut/console_color.hxx>
#include <ut/string_view.hxx>


enum class message_type
{
	error,
	warning,
	note,
	info,
	success,
	debug
};

using position = ::std::pair<::std::size_t, ::std::size_t>;

// (start, len)
using caret = position;


namespace internal
{
	ut::string_view get_line(ut::string_view, ::std::size_t);
	ut::string_view get_type_str(message_type);
	ut::console_color get_type_clr(message_type);
	void modify_counters(message_type);
}

void switch_debug(bool);

// 1 error and 2 warnings generated.
void post_summary();

void post_empty();

// internal compiler error
void post_ice(ut::string_view p_sender, ut::string_view p_message, bool p_abort = true);
void post_ice(ut::string_view p_sender, const ::std::exception& p_ex, bool p_abort = true);

// atfc: error: bla
void post_diagnostic(message_type p_type, ut::string_view p_sender, ut::string_view p_message);

// test.txt:1:4: error: bla
void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, ut::string_view p_message);

// test.txt:1:4: error: bla
//     blablablabla
//	   ^~~~~
void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, caret p_caret, ut::string_view p_src, ut::string_view p_message);

// test.txt:1:4: error: bla
//     blablablabla
//	   ^~~~~
//	   bla
void post_diagnostic(message_type p_type, ut::string_view p_sender, position p_pos, caret p_caret, ut::string_view p_src, ut::string_view p_fix, ut::string_view p_message);

