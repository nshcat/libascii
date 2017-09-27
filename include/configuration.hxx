#pragma once

#include <string>
#include <optional>
#include <type_traits>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "global_system.hxx"

namespace pt = boost::property_tree;

namespace internal
{
	// Adds reference to non-fundamental types
	template< typename T >
	using perfect_return_type_t = ::std::conditional_t<
		::std::is_fundamental_v<T>,
		T,
		::std::add_lvalue_reference_t<T>
	>;
}

class configuration
	: public global_system
{
	using tree_type = pt::ptree;
	using path_type = typename tree_type::path_type;
	
	public:
		auto initialize()
			-> void;
			
	public:
		auto tree()
			-> tree_type&;
		
		auto override_tree()
			-> tree_type&;
			
	public:
		// Write configuration data back to file.
		// This will only use the information available
		// in m_DataTree.
		auto save() const
			-> void;
			
	public:
		// We have to replicate most of the interface here since
		// we want to control which tree actually gets accessed
		// (and hereby implementing overrides)
		template< typename T >
		auto get(const path_type& p_path) const
			-> ::std::optional<T>
		{
			// Check if value has been overridden by 
			if(const auto t_result = m_OverrideTree.get_optional<T>(p_path);
				t_result)
			{
				return ::std::make_optional<T>(::std::move(*t_result));
			}
			else if(const auto t_result = m_DataTree.get_optional<T>(p_path);
				t_result)
			{
				return ::std::make_optional<T>(::std::move(*t_result));
			}
			else return { };
		}
		
	
	private:
		tree_type m_DataTree;		//< Tree containg configuration read from file
		tree_type m_OverrideTree;	//< Tree containg configuration read from command line.
									//  This separation is done to allow users to change
									//  configuration without messing it up with the overrides.	
};
