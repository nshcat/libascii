// TODO: Put internal implementation into .cxx file

#pragma once

#include <optional>
#include <cmath>
#include <string_view>
#include <stdexcept>
#include <ut/throwf.hxx>
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
	
	class area_impl
	{
		using position_type = screen_manager::position_type;
		using size_type = ::std::size_t;
		
		public:
			area_impl(const position_type& p_tl, const position_type& p_br)
				:	m_TL{p_tl},
					m_BR{p_br},
					m_Width{(p_br.x - p_tl.x) + 1},
					m_Height{(p_br.y - p_tl.y) + 1}
			{
				// This is fine since its guarantueed that p_br is actually
				// the bottom right corner
			}
		
		public:
			auto next() -> ::std::optional<position_type>
			{
				if(m_X >= m_Width)
				{
					m_X = 0; ++m_Y;				
				}
				
				if(m_Y >= m_Height) return { };
				
				const auto t_pos = m_TL + position_type{m_X, m_Y};
				++m_X;
				
				return ::std::make_optional(t_pos);
			}
		
		private:
			size_type m_Y{0U};
			size_type m_X{0U};
			const size_type m_Width;
			const size_type m_Height;
			const position_type m_TL;
			const position_type m_BR;
	};
	
	class rectangle_impl
	{
		using position_type = screen_manager::position_type;
		using size_type = ::std::size_t;
		
		public:
			rectangle_impl(const position_type& p_tl, const position_type& p_br)
				:	m_TL{p_tl},
					m_BR{p_br},
					m_Width{(p_br.x - p_tl.x) + 1},
					m_Height{(p_br.y - p_tl.y) + 1}
			{
				// This is fine since its guarantueed that p_br is actually
				// the bottom right corner
			}
		
		public:
			auto next() -> ::std::optional<position_type>
			{
				if(m_Y == 0 || m_Y == m_Height-1)
				{
					if(m_X >= m_Width)
					{
						m_X = 0; ++m_Y;				
					}
				}
				else
				{
					if(m_X > 0 && m_X < m_Width-1)
						m_X = m_Width-1;
					else if(m_X >= m_Width)
					{
						m_X = 0; ++m_Y;				
					}
				}
				
				if(m_Y >= m_Height) return { };
				
				const auto t_pos = m_TL + position_type{m_X, m_Y};
				++m_X;
				
				return ::std::make_optional(t_pos);
			}
		
		private:
			size_type m_Y{0U};
			size_type m_X{0U};
			const size_type m_Width;
			const size_type m_Height;
			const position_type m_TL;
			const position_type m_BR;
	};
	
	class line_impl
	{
		using position_type = screen_manager::position_type;
		using size_type = ::std::size_t;
		
		public:
			line_impl(const position_type& p_start, const position_type& p_end)
				:	m_Start{p_start},
					m_End{p_end}
			{
				m_DeltaX = m_End.x - m_Start.x;
				m_Ix = 0;
				{
					if(m_DeltaX > 0)
						m_Ix = 1;
					else if(m_DeltaX < 0)
						m_Ix = -1;
				}
				m_DeltaX = ::std::abs(m_DeltaX) << 1;
				
				m_DeltaY = m_End.y - m_Start.y;
				m_Iy = 0;
				{
					if(m_DeltaY > 0)
						m_Iy = 1;
					else if(m_DeltaY < 0)
						m_Iy = -1;
				}
				m_DeltaY = ::std::abs(m_DeltaY) << 1;
				
				if(m_DeltaX >= m_DeltaY)
				{
					m_Error = (m_DeltaY - (m_DeltaX >> 1));
				}
				else
				{
					m_Error = (m_DeltaX - (m_DeltaY >> 1));
				}
			}
		
		public:
			auto next() -> ::std::optional<position_type>
			{
				if(m_DeltaX >= m_DeltaY)
				{
					if(m_Start.x <= m_End.x)
					{
						const auto t_pos = m_Start;
					
						if((m_Error >= 0) && ((m_Error != 0) || (m_Ix > 0)))
						{
							m_Error -= m_DeltaX;
							m_Start.y += m_Iy;
						}
						
						m_Error += m_DeltaY;
						m_Start.x += m_Ix;
						
						return ::std::make_optional(t_pos);
					}
					else return { };
				}
				else
				{
					if(m_Start.y <= m_End.y)
					{
						const auto t_pos = m_Start;
					
						if((m_Error >= 0) && ((m_Error != 0) || (m_Iy > 0)))
						{
							m_Error -= m_DeltaY;
							m_Start.x += m_Ix;
						}
						
						m_Error += m_DeltaX;
						m_Start.y += m_Iy;
						
						return ::std::make_optional(t_pos);
					}
					else return { };
				}
			}
		
		private:
			position_type m_Start;
			position_type m_End;
			
			int m_DeltaX;
			int m_DeltaY;
			int m_Error;
			int m_Ix;
			int m_Iy;
	};
}

static auto point(const screen_manager::position_type& p_position)
	-> internal::point_impl
{
	return {p_position};
}

static auto area(const screen_manager::position_type& p_tl, const screen_manager::position_type& p_br)
	-> internal::area_impl
{
	if((p_tl.x > p_br.x) || (p_tl.y > p_br.y))
		ut::throwf<::std::runtime_error>("area: Invalid area formed by (%u, %u) and (%u, %u)",
			p_tl.x, p_tl.y, p_br.x, p_br.y);
	
	return {p_tl, p_br};
}

static auto rectangle(const screen_manager::position_type& p_tl, const screen_manager::position_type& p_br)
	-> internal::rectangle_impl
{
	if((p_tl.x > p_br.x) || (p_tl.y > p_br.y))
		ut::throwf<::std::runtime_error>("rectangle: Invalid rectangle formed by (%u, %u) and (%u, %u)",
			p_tl.x, p_tl.y, p_br.x, p_br.y);
	
	return {p_tl, p_br};
}

static auto line(const screen_manager::position_type& p_start, const screen_manager::position_type& p_end)
	-> internal::line_impl
{
	return {p_start, p_end};
}
