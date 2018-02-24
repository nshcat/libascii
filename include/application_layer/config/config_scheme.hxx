// TODO before commit: resolve all TODOs in this file

#pragma once

#include <stdexcept>
#include <vector>

#ifdef ROGUELIKE_FIXED_STL
#	include <variant_fixed>
#else
#	include <variant>
#endif

#include <boost/filesystem/operations.hpp>
#include <ut/throwf.hxx>
#include <ut/array_view.hxx>
#include "config_entry.hxx"

namespace application_layer::config
{
	namespace internal
	{
		//A variant type capable of holding all possible config_entry types.
		// This is defined outside of config_scheme to allow easy access for
		// the json de/serialization functions.
		using entry_variant = 	::std::variant<
									config_entry<bool>,
									config_entry<int>,
									config_entry<float>,
									config_entry<::std::string>	
								>;
								
		// TODO make this waaaay shorter by providing a default constructor and SETTERS for config_entry<T>
		template< typename T >
		void from_json_impl(const json& p_j, entry_variant& p_entry)
		{
			from_json_impl<T>(::std::conjunction<::std::is_arithmetic<T>, ::std::negation<::std::is_same<T, bool>>>{}, p_j, p_entry); 
		}
		
		template< typename T >
		void from_json_impl(::std::true_type, const json& p_j, entry_variant& p_entry)
		{
			// Arithmetic type
			const auto t_name = p_j.at("name").get<::std::string>();
			const auto t_path = p_j.at("path").get<::std::string>();
			const auto t_desc = p_j.at("description").get<::std::string>();
			const auto t_def = p_j.at("default").get<T>();
			const auto t_min = p_j.at("min").get<T>();	
			const auto t_max = p_j.at("max").get<T>();	
			
			if(p_j.count("mapping") && (p_j.at("mapping").count("long_name") || p_j.at("mapping").count("id")))
			{
				const auto t_mapping = p_j.at("mapping");
				
				if(t_mapping.count("long_name"))
				{
					p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def,
						t_min,
						t_max,
						t_mapping.at("long_name").get<::std::string>()
					};
				}
				else if(t_mapping.count("id"))
				{
					p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def,
						t_min,
						t_max,
						t_mapping.at("id").get<::std::size_t>()
					};
				}
			}
			else
			{
				p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def,
						t_min,
						t_max
					};
			}
		}
		
		template< typename T >
		void from_json_impl(::std::false_type, const json& p_j, entry_variant& p_entry)
		{
			const auto t_name = p_j.at("name").get<::std::string>();
			const auto t_path = p_j.at("path").get<::std::string>();
			const auto t_desc = p_j.at("description").get<::std::string>();
			const auto t_def = p_j.at("default").get<T>();
			
			if(p_j.count("mapping") && (p_j.at("mapping").count("long_name") || p_j.at("mapping").count("id")))
			{
				const auto t_mapping = p_j.at("mapping");
				
				if(t_mapping.count("long_name"))
				{
					p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def,
						t_mapping.at("long_name").get<::std::string>()
					};
				}
				else if(t_mapping.count("id"))
				{
					p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def,
						t_mapping.at("id").get<::std::size_t>()
					};
				}
			}
			else
			{
				p_entry = config_entry<T>{
						t_path,
						t_name,
						t_desc,
						t_def
					};
			}
		}
								
		void from_json(const json& p_j, entry_variant& p_entry);
	}
	
	
	// A class holding information about available configuration entries, like
	// name, data type, allowed value range and path in the configuration tree.
	class config_scheme
	{
		using container_type = ::std::vector<internal::entry_variant>;
		using view_type = ut::array_view<internal::entry_variant>;
		using const_view_type = ut::array_view<const internal::entry_variant>;
	
		public:
			using value_type = internal::entry_variant;
			using iterator = typename container_type::iterator;
			using const_iterator = typename container_type::const_iterator;
			using reverse_iterator = typename container_type::reverse_iterator;
			using const_reverse_iterator = typename container_type::const_reverse_iterator;
	
		public:
			// Empty scheme
			config_scheme() = default;
		
			// Construct from JSON document
			config_scheme(const boost::filesystem::path&);
			
			// Construct from range
			config_scheme(view_type);
			
		public:
			config_scheme(const config_scheme&) = default;
			config_scheme(config_scheme&&) = default;
			
			config_scheme& operator=(const config_scheme&) = default;
			config_scheme& operator=(config_scheme&&) = default;
			
		public:
			auto view() const
				-> const_view_type;
			
		public:
			auto begin()
				-> iterator;
				
			auto begin() const
				-> const_iterator;	
		
			auto cbegin() const
				-> const_iterator;
				
			auto end()
				-> iterator;
				
			auto end() const
				-> const_iterator;	
		
			auto cend() const
				-> const_iterator;
				
			auto rbegin()
				-> reverse_iterator;
				
			auto rbegin() const
				-> const_reverse_iterator;	
		
			auto crbegin() const
				-> const_reverse_iterator;
				
			auto rend()
				-> reverse_iterator;
				
			auto rend() const
				-> const_reverse_iterator;	
		
			auto crend() const
				-> const_reverse_iterator;
	
		private:
			container_type m_Entries{ };
	};  
}
