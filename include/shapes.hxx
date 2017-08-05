#pragma once

#include <optional>
#include <string_view>
#include <screen.hxx>

namespace internal
{
	class point_impl
	{
		using position_type = screen_manager::position_type;
	
		public:
			point_impl(const position_type& p_pos)
				: m_Pos{p_pos}
			{
			}
	
		public:
			auto next() -> ::std::optional<position_type>
			{
				if(m_First)
				{
					m_First = false;
					return ::std::make_optional(m_Pos);
				}
				else return {};
			}
		
		private:
			bool m_First{true};
			position_type m_Pos;
	};
}

static auto point(const screen_manager::position_type& p_position)
	-> internal::point_impl
{
	return {p_position};
}
