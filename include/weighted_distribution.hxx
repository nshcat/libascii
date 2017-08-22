#pragma once

#include <array>
#include <type_traits>
#include <random>
#include <utility>
#include <vector>
#include <stdexcept>
#include <initializer_list>
#include <ut/type_traits.hxx>

template< typename T >
class weighted_distribution
{
	using pair_type = ::std::pair<T, float>;
	using list_type = ::std::initializer_list<pair_type>;

	public:
		weighted_distribution(const list_type& p_list)
			: m_Data(p_list.begin(), p_list.end())
		{
			
		}
		
	public:
		template< typename TGenerator >
		auto operator()(TGenerator& p_gen)
			-> const T&
		{
			if(m_Data.size() == 0)
				throw ::std::runtime_error("weighted_distribution::operator(): No entries!");
		
			// Roll random number
			const auto t_roll = m_Distrib(p_gen);
			
			float t_sum{ };
			for(const auto& t_pair: m_Data)
			{
				t_sum += t_pair.second;
				
				if(t_roll <= t_sum)
					return t_pair.first;							
			}
			
			// If the propabilities didn't add up to 1 we could reach this:
			// The generated float value is outside of our range!
			// We just return the last element here, aka we are extending
			// the last propability to sum up to 1.
			return ::std::prev(m_Data.end())->first;
		}
	
	private:
		::std::vector<pair_type> m_Data;
		::std::uniform_real_distribution<float> m_Distrib{0.f, 1.f};
};
