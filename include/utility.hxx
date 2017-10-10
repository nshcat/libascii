#pragma once

#include <algorithm>

namespace internal
{
	template<	typename TTarget,
				typename TValue,
				TValue TTarget::* Ptr,
				typename TUniqueTag
	>
	struct tag_base
	{
		using target_type = TTarget;
		using value_type = TValue;
	
		tag_base(const TValue& p_value)
			: m_Value{p_value}
		{
		}
	
		void apply(TTarget& p_target)
		{
			p_target.*Ptr = m_Value;
		}
	
		TValue m_Value;
	};	
}

// TContainer has to be a sequential container
template< typename TContainer, typename TPred >
auto insert_sorted(TContainer& p_container, typename TContainer::const_reference p_item, TPred p_pred)
	-> typename TContainer::iterator
{
	return p_container.insert(
		::std::upper_bound(::std::begin(p_container), ::std::end(p_container), p_item, p_pred),
		p_item
	);
}

template< typename TIt, typename T, typename TComp = ::std::less<>>
auto binary_find(TIt p_first, TIt p_last, const T& p_val, TComp p_comp = {})
	-> TIt
{
	p_first = ::std::lower_bound(p_first, p_last, p_val, p_comp);
	return p_first != p_last && !p_comp(p_val, *p_first) ? p_first : p_last;
};		
