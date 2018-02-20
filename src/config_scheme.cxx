#include <stdexcept>
#include <fstream>
#include <ut/throwf.hxx>
#include <nlohmann/json.hpp>
#include <application_layer/config/config_scheme.hxx>

using json = nlohmann::json;

namespace application_layer::config
{
	namespace internal
	{
		void from_json(const json& p_j, entry_variant& p_entry)
		{
			static const ::std::map<::std::string, ::std::function<void(const json&, entry_variant&)>> t_map {
				{ "bool", [](const json& p_json, entry_variant& p_entry) { from_json_impl<bool>(p_json, p_entry); } },
				{ "int", [](const json& p_json, entry_variant& p_entry) { from_json_impl<int>(p_json, p_entry); } },
				{ "float", [](const json& p_json, entry_variant& p_entry) { from_json_impl<float>(p_json, p_entry); } },
				{ "string", [](const json& p_json, entry_variant& p_entry) { from_json_impl<::std::string>(p_json, p_entry); } }
			};
		
			const auto t_type = p_j.at("type").get<::std::string>();
			
			if(!t_map.count(t_type))
				ut::throwf<::std::runtime_error>("invalid config entry type: \"%s\"", t_type);
			else
				t_map.at(t_type)(p_j, p_entry);
			
		}
	}
	
	config_scheme::config_scheme(const boost::filesystem::path& p_path)
	{
		// Check if it exists
		if(!boost::filesystem::exists(p_path))
			throw ::std::runtime_error("config_scheme: Could not open configuration scheme file: Does not exist");
		
		// Check that it isnt a directory
		if(boost::filesystem::is_directory(p_path))
			throw ::std::runtime_error("config_scheme: Could not open configuration scheme file: Is a directory");
	
		try
		{
			// Load json document
			::std::ifstream t_file{ p_path.string() };			
			json t_j;
			t_file >> t_j;
			
			// The root needs to be an array
			if(t_j.is_array())
			{
				// Try to parse each entry as a config_entry variant
				for(const auto& t_elem: t_j)
				{
					internal::entry_variant t_entry = t_elem;
					m_Entries.push_back(t_entry);
				}
			}
			else throw ::std::runtime_error("root element needs to be an array");
			
		}
		catch(const ::std::exception& p_ex)
		{
			ut::throwf<::std::runtime_error>("config_scheme: failed to load json file \"%s\": %s",
				p_path.string(),
				p_ex.what()
			);
		}
	}
	
	config_scheme::config_scheme(view_type p_range)
		: m_Entries{ p_range.begin(), p_range.end() }
	{
		
	}
	
	auto config_scheme::view() const
		-> const_view_type
	{
		return { m_Entries.begin(), m_Entries.end() };
	}
	
	auto config_scheme::begin()
		-> iterator
	{
		return m_Entries.begin();
	}
	
	auto config_scheme::begin() const
		-> const_iterator
	{
		return m_Entries.cbegin();
	}
	
	auto config_scheme::cbegin() const
		-> const_iterator
	{
		return m_Entries.cbegin();
	}
	
	auto config_scheme::end()
		-> iterator
	{
		return m_Entries.end();
	}
	
	auto config_scheme::end() const
		-> const_iterator
	{
		return m_Entries.cend();
	}
	
	auto config_scheme::cend() const
		-> const_iterator
	{
		return m_Entries.cend();
	}
	
	auto config_scheme::rbegin()
		-> reverse_iterator
	{
		return m_Entries.rbegin();
	}
	
	auto config_scheme::rbegin() const
		-> const_reverse_iterator
	{
		return m_Entries.crbegin();
	}
	
	auto config_scheme::crbegin() const
		-> const_reverse_iterator
	{
		return m_Entries.crbegin();
	}
	
	auto config_scheme::rend()
		-> reverse_iterator
	{
		return m_Entries.rend();
	}
	
	auto config_scheme::rend() const
		-> const_reverse_iterator
	{
		return m_Entries.crend();
	}
	
	auto config_scheme::crend() const
		-> const_reverse_iterator
	{
		return m_Entries.crend();
	}
}
