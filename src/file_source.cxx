#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <application_layer/config/file_source.hxx>
#include <application_layer/config/ptree_merge.hxx>

namespace application_layer::config
{
	file_source::file_source(const boost::filesystem::path& p_path)
		: m_Path{p_path}
	{
	}
	
	auto file_source::populate(boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree&
	{
		// Check for file existance
		if(!boost::filesystem::exists(m_Path))
			throw ::std::runtime_error("file_source: Could not open configuration file: File does not exist");
			
		// Check for correct type
		if(boost::filesystem::is_directory(m_Path))
			throw ::std::runtime_error("file_source: Could not open configuration file: Is a directory");
	
		// Create tree from JSON file
		boost::property_tree::ptree t_tree{ };
		boost::property_tree::read_json(m_Path.string(), t_tree);
		
		// Merge into given tree
		return internal::merge(p_src, t_tree);
	}
}
