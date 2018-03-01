#pragma once

#include <type_traits>
#include <functional>
#include <ut/type_traits.hxx>

#include "utility.hxx"

namespace utility::pnfa
{
	namespace internal
	{
		// TODO allow custom comparers
		template< typename Tinput >
		class match_t
		{
			public:
				match_t(const Tinput& p_in)
					: m_Val{p_in}
				{
				}
				
				match_t(Tinput&& p_in)
					: m_Val{::std::move(p_in)}
				{
				}
				
			public:
				template< typename... Ts >
				auto operator()(const Tinput& p_in, Ts&&...)
					-> bool
				{
					return p_in == m_Val;
				}
			
			protected:
				Tinput m_Val;
		};
		
		class always_true_t
		{			
			public:
				template< typename... Ts >
				auto operator()(Ts&&...)
					-> bool
				{
					return true;
				}
		};
	}

	template< typename Tinput >
	auto match(const Tinput& p_in)
		-> internal::match_t<Tinput>
	{
		return { p_in };
	}
	
	template< typename Tinput >
	auto match(Tinput&& p_in)
		-> internal::match_t<Tinput>
	{
		return { ::std::move(p_in) };
	}
	
	auto always_true()
		-> internal::always_true_t
	{
		return { };
	}
}
