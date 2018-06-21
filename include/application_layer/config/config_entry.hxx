#pragma once

#include <type_traits>
#include <string>
#include <optional>
#include <stdexcept>
#include <limits>

#ifdef ROGUELIKE_FIXED_STL
#	include <variant_fixed>
#else
#	include <variant>
#endif

#include <ut/throwf.hxx>
#include <cl.hxx>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace application_layer::config
{
	namespace internal
	{
		// We only support string, int, float and bool.
		template< typename T >
		struct is_valid_entry_type
			: 	::std::disjunction<
					::std::is_same<T, int>,
					::std::is_same<T, float>,
					::std::is_same<T, bool>,
					::std::is_same<T, ::std::string>
				>
		{
		};
		
		template< typename T >
		using is_int = ::std::is_same<T, int>;
		
		template< typename T >
		using is_float = ::std::is_same<T, float>;
		
		template< typename T >
		using is_bool = ::std::is_same<T, bool>;
		
		template< typename T >
		using is_string = ::std::is_same<T, ::std::string>;
		
		template< typename T >
		using cl_argument_type_for_t =
			ut::category_of_t<
				T,
				ut::category<cl::integer_argument<int>, is_int>,
				ut::category<cl::string_argument, is_string>,
				ut::category<cl::boolean_argument, is_bool>,
				ut::category<cl::floating_argument<float>, is_float>
			>;
		
		
		template< typename T >
		constexpr bool is_valid_entry_type_v = is_valid_entry_type<T>::value;
		
		class mapping
		{
			public:	
				mapping(const ::std::string& p_c, const ::std::string& p_ln, const ::std::optional<char>& p_sn)
					: m_Category{p_c}, m_LongName{p_ln}, m_ShortName{p_sn}
				{
				}
			
			public:
				auto category() const
					-> const ::std::string&
				{
					return m_Category;
				}
				
				auto long_name() const
					-> const ::std::string&
				{
					return m_LongName;
				}
				
				auto short_name() const
					-> const ::std::optional<char>&
				{
					return m_ShortName;
				}
		
			private:
				::std::string m_Category;
				::std::string m_LongName;
				::std::optional<char> m_ShortName;
		};
				
		template< typename T >
		class entry_base
		{
			protected:
				using mapping_type = mapping;
				
			public:
				using value_type = T;
			
			public:
				entry_base(const ::std::string& p_path,
				           const ::std::string& p_name,
				           const ::std::string& p_desc,
				           const T& p_default)
					: m_Path{p_path}, m_Name{p_name}, m_Description{p_desc}, m_Default{p_default}, m_Mapping{}
				{
				}
				
				entry_base(const ::std::string& p_path,
				           const ::std::string& p_name,
				           const ::std::string& p_desc,
				           const T& p_default,
				           const mapping_type& p_map)
					: m_Path{p_path}, m_Name{p_name}, m_Description{p_desc}, m_Default{p_default}, m_Mapping{p_map}
				{
				}
		
				entry_base()
				{
				}
		
			public:
				auto path() const
					-> const ::std::string&
				{
					return m_Path;
				}
				
				auto mapping() const
					-> const ::std::optional<mapping_type>&
				{
					return m_Mapping;
				}
				
				auto name() const
					-> const ::std::string&
				{
					return m_Name;
				}
				
				auto description() const
					-> const ::std::string&
				{
					return m_Description;
				}
				
				auto default_value() const
					-> const T&
				{
					return m_Default;
				}
		
			protected:
				::std::string m_Path{ };			//< Actual absolute path in the property tree / json document
				::std::string m_Name{ };			//< Fancy name shown to the user
				::std::string m_Description{ };		//< Description shown to the user
				::std::optional<mapping_type> m_Mapping{ };			//< Optional command line argument mapping
				T m_Default{ };						//< Default value
		};

		template< typename T >
		class arithmetic_entry
			: public entry_base<T>
		{
			using base_type = entry_base<T>;
		
			public:
				arithmetic_entry(const ::std::string& p_path,
				           		 const ::std::string& p_name,
				           	     const ::std::string& p_desc,
				           		 T p_default,
				           		 T p_min,
				           		 T p_max)
					: base_type{p_path, p_name, p_desc, p_default}, m_Min{p_min}, m_Max{p_max}
				{
					check_bounds();
				}
				
				arithmetic_entry(const ::std::string& p_path,
				           		 const ::std::string& p_name,
				           	     const ::std::string& p_desc,
				           		 T p_default,
				           		 T p_min,
				           		 T p_max,
				           		 const typename base_type::mapping_type& p_map)
					: base_type{p_path, p_name, p_desc, p_default, p_map}, m_Min{p_min}, m_Max{p_max}
				{
					check_bounds();
				}
				
				arithmetic_entry()
				{
					
				}
		
			public:
				auto max() const
					-> T
				{
					return m_Max;
				}
				
				auto min() const
					-> T
				{
					return m_Min;
				}
				
			protected:
				// Check if the given bounds (min and max value) are legal.
				// This means that min < max.
				auto check_bounds() const
					-> void
				{
					if(m_Min >= m_Max)
					{
						ut::throwf<::std::runtime_error>(
							"could not read scheme entry for \"%s\": invalid bounds given (min >= max)",
							this->m_Name
						);
					}
						
				}
		
			protected:
				T m_Max{ ::std::numeric_limits<T>::max() };
				T m_Min{ ::std::numeric_limits<T>::min() };
		};
	}

	template< typename T >
	class config_entry
		: public ::std::conditional_t<::std::is_arithmetic_v<T> && not ::std::is_same_v<T, bool>, internal::arithmetic_entry<T>, internal::entry_base<T>> 
	{
		using base_type = ::std::conditional_t<::std::is_arithmetic_v<T> && not ::std::is_same_v<T, bool>, internal::arithmetic_entry<T>, internal::entry_base<T>>;
	
		// Only int, float and ::std::string are allowed for now.
		static_assert(internal::is_valid_entry_type_v<T>,
			"config_entry<T> only supports int, float, bool and ::std::string");
	
		public:
			// Inherit constructor
			using base_type::base_type;		
	};
}
