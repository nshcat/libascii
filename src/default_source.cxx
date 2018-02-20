#include <application_layer/config/default_source.hxx>


namespace application_layer::config
{
	default_source::default_source(const config_scheme& p_scheme)
		: m_Scheme{p_scheme}
	{
	}
	
	auto default_source::populate(boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree&
	{
		// Iterate through all configuration entries
		for(const auto& t_entry: m_Scheme)
		{
			::std::visit(
				[&p_src](auto t_elem) -> void
				{
					p_src.put(t_elem.path(), t_elem.default_value());
				},
				t_entry
			);
		}
	
		return p_src;
	}
}
