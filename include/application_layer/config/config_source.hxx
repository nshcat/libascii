#pragma once

#include <boost/property_tree/ptree.hpp>

namespace application_layer::config
{
	class config_source
	{
		public:
			virtual auto populate(boost::property_tree::ptree&) -> boost::property_tree::ptree& = 0;
	};
}
