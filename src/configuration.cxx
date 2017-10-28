#include <string>
#include <stdexcept>
#include <boost/filesystem/operations.hpp>

#include <configuration.hxx>
#include <global_state.hxx>

auto configuration::initialize()
	-> void
{
	// Retrieve config file path
	const auto t_path = global_state<path_manager>().config_path();
	
	// Check if the file exists. If not we need to
	// restore the defaults.
	if(boost::filesystem::exists(t_path))
	{
		// Check that it is not, in fact, a directory.
		if(boost::filesystem::is_directory(t_path))
			throw ::std::runtime_error("configuration: Could not open configuration file: Is a directory");
			
		// Load the property tree from file
		pt::read_json(t_path.string(), m_DataTree);
	}
	else
	{
		// Create new configuration file populated with default values
		reset();
	}
}

auto configuration::reset()
	-> void
{
	// Retrieve data path
	const auto t_dataPath = global_state<path_manager>().data_path();
	
	// Load property tree from default config file
	pt::read_json((t_dataPath / "config" / "default.json").string(), m_DataTree);
	
	// Write back to file
	save();
}

auto configuration::save() const
	-> void
{
	const auto t_path = global_state<path_manager>().config_path();
	
	pt::write_json(t_path.string(), m_DataTree);
}
