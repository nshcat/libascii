#pragma once

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
