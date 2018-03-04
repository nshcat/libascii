#pragma once

#include <memory>
#include <log.hxx>
#include "global_system.hxx"
#include "commandline.hxx"


class log_manager	//< Manage liblog targets and configuration
	: public global_system
{
	using console_target = lg::console_target<lg::clang_formatter>;
	using file_target = lg::file_target<>;
	
	public:
		auto initialize()
			-> void;
			
		auto shutdown()
			-> void;

	private:
		::std::unique_ptr<console_target> m_Console{ };
		::std::unique_ptr<file_target> m_File{ };
};
