#pragma once

#include "config_source.hxx"
#include "config_scheme.hxx"

namespace application_layer::config
{
	// A configuration source that uses a configuration scheme to retrieve
	// default values for all config entries
	class default_source
		: public config_source
	{
		public:
			default_source(const config_scheme& p_scheme);
			
			default_source(const default_source&) = default;
			default_source(default_source&&) = default;
			
			default_source& operator=(const default_source&) = default;
			default_source& operator=(default_source&&) = default;
	
		public:
			virtual auto populate(boost::property_tree::ptree&)
				-> boost::property_tree::ptree& override;
				
		protected:
			config_scheme m_Scheme{ };	//< We do store a copy of the scheme
	};
}
