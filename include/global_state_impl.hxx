#pragma once

#include <type_traits>
#include <initializer_list>
#include <tuple>
#include <utility>
#include <ut/type_traits.hxx>

#include "global_system.hxx"

namespace internal
{
	// Reverse a index_sequence containing an index interval.
	// This only works on intervals of the form [0, N] or [N, 0],
	// aka. no gaps and no duplicate elements.
	template< typename T >
	struct index_list_reverser;
	
	template< typename T, T... Ts >
	struct index_list_reverser <::std::integer_sequence<T, Ts...>>
	{
		using type = ::std::integer_sequence<T, ((sizeof...(Ts)-1)-Ts)...>;
	};
	
	template< typename T >
	using reversed = typename index_list_reverser<T>::type;
	//
	
	
	template< typename... Ts >
	class global_state_impl
	{
		// TODO assert that all types are unique
	
		// Assert that all given types are actually global systems
		static_assert(::std::conjunction_v<is_global_system<Ts>...>,
			"global_state_impl: All given types have to adhere to the"
			"global_state concept!");
	
		using tuple_type = ::std::tuple<Ts...>;
		using index_type = ::std::index_sequence_for<Ts...>;
		using reverse_index_type = reversed<index_type>;
		
		public:
			global_state_impl() = default;
			
		public:
			global_state_impl(const global_state_impl&) = delete;
			global_state_impl(global_state_impl&&) = delete;
			
			global_state_impl& operator=(const global_state_impl&) = delete;
			global_state_impl& operator=(global_state_impl&&) = delete;
			
		public:
			auto initialize()
				-> void
			{
				initialize_impl(index_type{});
			}
			
			auto shutdown()
				-> void
			{
				shutdown_impl(reverse_index_type{});
			}
			
		public:
			auto systems()
				-> tuple_type&
			{
				return m_Systems;
			}
			
		private:
			template< ::std::size_t... Ns >
			auto initialize_impl(::std::index_sequence<Ns...>)
				-> void
			{
				auto x = { (::std::get<Ns>(m_Systems).initialize(), 0)... };
			}
			
			template< ::std::size_t... Ns >
			auto shutdown_impl(::std::index_sequence<Ns...>)
				-> void
			{
				auto x = { (::std::get<Ns>(m_Systems).shutdown(), 0)... };
			}
			
		private:
			tuple_type m_Systems;	//< This is default_constructible, since all global
									//  systems are required to be trivially default
									//  constructible
	};
}
