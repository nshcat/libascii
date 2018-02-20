#pragma once

#include <boost/filesystem/operations.hpp>
#include "config_source.hxx"
#include "config_scheme.hxx"

namespace application_layer::config
{
	// A configuration source that loads configuration data from
	// a json file.
	class file_source
		: public config_source
	{
		public:
			file_source(const boost::filesystem::path& p_path);
			
			file_source(const file_source&) = default;
			file_source(file_source&&) = default;
			
			file_source& operator=(const file_source&) = default;
			file_source& operator=(file_source&&) = default;
	
		public:
			virtual auto populate(boost::property_tree::ptree&)
				-> boost::property_tree::ptree& override;
				
		protected:
			boost::filesystem::path m_Path{ };	//< Path to configuration file. We could retrieve this from the path
												//  manager, but we want to keep dependencies between classes as small
												//	as possible.
	};
}
