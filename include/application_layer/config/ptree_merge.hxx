#pragma once

#include <boost/property_tree/ptree.hpp>

namespace application_layer::config::internal
{
	// Merges given source tree into the given destination tree.
	// Already existing keys are overridden.
	// Returns a reference to the destination tree.
	auto merge(boost::property_tree::ptree& p_dest, const boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree&;
		
	// This overload will copy the destination tree to be able to modify it.
	// A copy of the modified instance is returned.
	auto merge(const boost::property_tree::ptree& p_dest, const boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree;
}
