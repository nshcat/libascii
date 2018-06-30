#pragma once

#include <string>

class game_info
{
	public:
		game_info(const ::std::string& p_name, const ::std::string& p_version, const ::std::string& p_desc)
			: m_Name{p_name}, m_Version{p_version}, m_Description{p_desc}
		{
		}
	
	public:
		auto name() const
			-> const ::std::string&;
			
		auto version() const
			-> const ::std::string&;
			
		auto description() const
			-> const ::std::string&;

	protected:
		::std::string m_Name;			//< Title of the game
		::std::string m_Version;		//< Current version of the game
		::std::string m_Description;	//< Short description. This is shown as part of the command line help
};
