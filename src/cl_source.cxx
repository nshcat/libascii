#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <application_layer/config/cl_source.hxx>


namespace application_layer::config
{
	cl_source::cl_source(const config_scheme& p_scheme, const cl::handler& p_handler)
		: m_Scheme{p_scheme}, m_Handler{p_handler}
	{
	}
	
	auto cl_source::populate(boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree&
	{
		// Iterate through all configuration entries
		for(const auto& t_entry: m_Scheme)
		{
			::std::visit(
				[&p_src, this](auto t_elem) -> void	//< Note: t_elem is always of type `configuration_entry<T>`, where T is unknown.
				{
					// We need to painfully extract the actual type of configuration value we
					// are dealing with here. To our luck, `configuration_entry<T>` defines
					// a public type alias called `value_type` that is always set to `T`.
					// This makes our work a lot easier here.
					using value_type = typename ::std::decay_t<decltype(t_elem)>::value_type;
					
					// We dont really care about the particular command line handler that is
					// used to parse the value for this configuration entry, we just require
					// it to be able to parse the correct type, signaled by being derived
					// from `cl::value_base<T>`, where T is our type.
					using handler_type = cl::internal::value_base<value_type>;
				
					// We can only get values from the command line if a mapping exists
					if(t_elem.mapping())
					{
						// TODO solve without try-catch. `cl::command_base` offers `has() -> bool`.
						try
						{
							// Obtain pointer to argument that handles this particular configuration entry.
							// This could either be by long name, or by ID number.
							const auto t_arg = ::std::visit(
								[this](auto t_key)
								{
									return m_Handler.get(t_key);
								},
								*t_elem.mapping()
							);
							
							// For obvious reasons, the argument we get needs to be able to
							// parse values of the type we are dealing with in this configuration
							// entry.
							if(!t_arg->template is<handler_type>())
								throw ::std::runtime_error("incorrect argument handler type");							
						
							// Write value to tree.
							p_src.put(t_elem.path(), t_arg->template as<handler_type>()->value());
						}
						catch(const ::std::exception& p_ex)
						{
							::std::cout << "cl_source: error: could not load value for config entry \""
										<< t_elem.name()
										<< "\": "
										<< p_ex.what()
										<< ::std::endl;
						}
					}
				},
				t_entry
			);
		}
	
		return p_src;
	}
}
