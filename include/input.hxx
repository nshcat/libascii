#pragma once

#include <string>

class input_manager
{
	public:
		void initialize();
		
	public:
		bool has_text() const;
		::std::string text();
		
		void begin_text();
		void end_text();
		
	public:
		
};
