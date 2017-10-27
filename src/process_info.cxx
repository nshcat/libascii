#include <process_info.hxx>

auto process_info::title() const
	-> const ::std::string&
{
	return m_Title;
}

auto process_info::description() const
	-> const ::std::string&
{
	return m_Description;
}
