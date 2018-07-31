#include <game_info.hxx>

auto game_info::name() const
	-> const ::std::string&
{
	return m_Name;
}		

auto game_info::version() const
	-> const ::std::string&
{
	return m_Version;
}
	
auto game_info::description() const
	-> const ::std::string&
{
	return m_Description;
}

auto game_info::window_title() const
	-> const ::std::string&
{
	return m_WindowTitle;
}
