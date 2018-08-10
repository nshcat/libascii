#include <string>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <boost/filesystem/operations.hpp>
#include <ut/utility.hxx>

#include <configuration.hxx>
#include <commandline.hxx>
#include <global_state.hxx>

#include <application_layer/config/config_entry.hxx>
#include <application_layer/config/cl_source.hxx>
#include <application_layer/config/default_source.hxx>
#include <application_layer/config/file_source.hxx>

using namespace application_layer::config;
using namespace application_layer::config::internal;

auto configuration::tree()
	-> tree_type&
{
	return m_DataTree;
}

auto configuration::override_tree() const
	-> const tree_type&
{
	return m_OverrideTree;
}

auto configuration::scheme() const
	-> const scheme_type&
{
	return m_CfgScheme;
}

auto configuration::initialize()
	-> void
{
	// Retrieve configuration scheme file path
	const auto t_schemepath = global_state<path_manager>().data_path() / "config" / "scheme.json";
	
	// Load scheme
	m_CfgScheme = scheme_type{ t_schemepath };
	
	// Populate data tree with default settings. This way missing entries
	// in the configuration file will fall back to default values.
	default_source t_defSrc{ scheme() };
	t_defSrc.populate(m_DataTree);

	// Retrieve config file path
	const auto t_path = global_state<path_manager>().config_path();
	
	// Check if the file exists. We only want to try to load it
	// if it actually exists.
	if(boost::filesystem::exists(t_path))
	{
		file_source t_fileSrc{ t_path };
		t_fileSrc.populate(m_DataTree);
	}
	
	// Perform bounds checking on all configuration entries
	check_bounds();
	
	// Save back the configuration file. This makes sure that missing values
	// are replaced with their defaults.
	save();
	
	// Add all entries to the command line handler in the form of arguments, for which
	// a mapping was defined
	populate_cl_arguments();
}

auto configuration::populate_overrides()
	-> void
{
	// Load values from the command line. These are stored in a separate
	// tree to avoid writing them out to disk. They are only meant as a temporary
	// overwrite.
	cl_source t_clSrc{ scheme(), g_clHandler };
	t_clSrc.populate(m_OverrideTree);
}

auto configuration::save() const
	-> void
{
	const auto t_path = global_state<path_manager>().config_path();
	
	pt::write_json(t_path.string(), m_DataTree);
}

auto configuration::populate_cl_arguments()
	-> void
{
	// Check all entries
	for(const auto& t_entry: scheme())
	{
		// The stored value is always of type `config_entry<T>`, where `T` is unknown to
		// us at this point.
		::std::visit(
			[this](auto t_elem) -> void
			{
				using elem_type = ::std::decay_t<decltype(t_elem)>;
				using value_type = typename elem_type::value_type;
				using argument_type = cl_argument_type_for_t<value_type>;
				
				// We are only interested in entries that have a mapping defined for them
				if(!t_elem.mapping())
					return;
				
				// Build command line argument
				
				// The entry has min/max defined
				if constexpr(::std::is_base_of_v<arithmetic_entry<value_type>, elem_type>)
				{
					auto t_arg = (t_elem.mapping()->short_name() ?
						::std::make_unique<argument_type>(
							cl::default_value(t_elem.default_value()),
							cl::min(t_elem.min()),
							cl::max(t_elem.max()),
							cl::description(t_elem.description()),
							cl::long_name(t_elem.mapping()->long_name()),
							cl::category(t_elem.mapping()->category()),
							cl::short_name(t_elem.mapping()->short_name().value())
						)
						:
						::std::make_unique<argument_type>(
							cl::default_value(t_elem.default_value()),
							cl::min(t_elem.min()),
							cl::max(t_elem.max()),
							cl::description(t_elem.description()),
							cl::long_name(t_elem.mapping()->long_name()),
							cl::category(t_elem.mapping()->category())
						));
						
					// Insert into handler
					g_clHandler.add(::std::move(t_arg));
				}
				else
				{
					auto t_arg = (t_elem.mapping()->short_name() ?
						::std::make_unique<argument_type>(
							cl::default_value(t_elem.default_value()),
							cl::description(t_elem.description()),
							cl::long_name(t_elem.mapping()->long_name()),
							cl::category(t_elem.mapping()->category()),
							cl::short_name(t_elem.mapping()->short_name().value())
						)
						:
						::std::make_unique<argument_type>(
							cl::default_value(t_elem.default_value()),
							cl::description(t_elem.description()),
							cl::long_name(t_elem.mapping()->long_name()),
							cl::category(t_elem.mapping()->category())
						));
						
					// Insert into handler
					g_clHandler.add(::std::move(t_arg));
				}
				
				
			},
			t_entry
		);	
	}
}

auto configuration::check_bounds()
	-> void
{
	// Check all config entries described by the scheme
	for(const auto& t_entry: scheme())
	{
		// The stored value is always of type `config_entry<T>`, where `T` is unknown to
		// us at this point.
		::std::visit(
			[this](auto t_elem) -> void
			{
				using elem_type = ::std::decay_t<decltype(t_elem)>;
				using value_type = typename elem_type::value_type;
				
				// We are only interested in values that have min/max values defined for them
				if constexpr(::std::is_base_of_v<arithmetic_entry<value_type>, elem_type>)
				{
					// Retrieve actual value from the tree.
					// This operation is guarantueed to succeed.
					const auto t_val = *tree().get_optional<value_type>(t_elem.path());
				
					// Check if given value is in range given by configuration scheme
					if(!ut::in_range<value_type>(t_val, t_elem.min(), t_elem.max()))
					{
						::std::cout << "configuration: error: Given value for \""
								  	<< t_elem.name() << "\" (" << t_val
								  	<< ") is out of range ["
								  	<< t_elem.min() << ", " << t_elem.max() << "]. Using default value instead"
								  	<< ::std::endl;
								  	
						tree().put(t_elem.path(), t_elem.default_value());
					}
				}
			},
			t_entry
		);
	}
}
