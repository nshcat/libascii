#pragma once

#include <array>
#include <type_traits>
#include <random>
#include <utility>
#include <stdexcept>
#include <initializer_list>

template< typename T, ::std::size_t N = 100UL >
class weighted_collection
{
	using pair_type = ::std::pair<T, float>;
	using list_type = ::std::initializer_list<pair_type>;

	public:
		weighted_collection(list_type p_list)
		{
			// Check that total probability does not exceed 1
			const auto t_sum = ::std::accumulate(p_list.begin(), p_list.end(), 0.f,
				[](const float& p_a, const pair_type& p_b)
					-> float
				{
					return p_a + p_b.second;
				}
			);
			
			if(t_sum > 1.f)
				throw ::std::runtime_error("weighted_collection: Total probability cannot exceed 1!");
				
			auto t_iter = m_Data.begin();
			for(const auto& t_pair: p_list)
			{
				const ::std::size_t t_count = t_pair.second * N;
				
				t_iter = ::std::fill_n(t_iter, t_count, t_pair.first);
			}
			
			if(t_iter != m_Data.end())
				::std::fill(t_iter, m_Data.end(), T{});
		}
	
	public:
		template< typename TGenerator >
		auto operator()(TGenerator& p_gen)
			-> const T&
		{
			return m_Data[m_Distrib(p_gen)];
		}
		
		template< typename TGenerator >
		auto shuffle(TGenerator& p_gen)
			-> void
		{
			::std::shuffle(m_Data.begin(), m_Data.end(), p_gen);
		}
	
	private:
		::std::array<T, N> m_Data;
		::std::uniform_int_distribution<::std::size_t> m_Distrib{0, N-1};
};

