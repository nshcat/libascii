#pragma once

#include <string>

// Class holding extended information about a process.
class process_info
{
	public:
		process_info() = default;
		
		process_info(const ::std::string& p_title, const ::std::string& p_desc)
			: m_Title{p_title}, m_Description{p_desc}
		{
		}
		
	public:
		process_info(const process_info&) = default;
		process_info(process_info&&) = default;
		
		process_info& operator=(const process_info&) = default;
		process_info& operator=(process_info&&) = default;
		
	public:
		auto title() const
			-> const ::std::string&;
			
		auto description() const
			-> const ::std::string&;

	private:
		const ::std::string m_Title;
		const ::std::string m_Description;
};
