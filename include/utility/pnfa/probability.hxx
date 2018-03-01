#pragma once

#include <stdexcept>
#include <ut/utility.hxx>

namespace utility::pnfa
{
	namespace internal
	{
		// A wrapper type used to provide type safety to the public interface.
		// It basically just stores a probability value in the interval [0, 1].
		class probability
		{
			using value_type = double;		
			
			public:
				probability(value_type p_val)
					: m_Val{p_val}
				{
					if(!ut::in_range<value_type>(p_val, value_type{ }, value_type{1.0}))
						throw ::std::runtime_error("probability: Probability out of range [0, 1]");
				}
		
			public:
				auto value() const
					-> value_type
				{
					return m_Val;
				}
		
			protected:
				value_type m_Val{ };
		};
	}
	
	auto percentage(double p_val)
		-> internal::probability
	{
		if(!ut::in_range(p_val, 0., 100.))
			throw ::std::runtime_error("percent: Percentage out of range [0, 100]");
		
		return { p_val / 100. };
	}
	
	auto probability(double p_val)
		-> internal::probability
	{
		return { p_val };
	}
}
