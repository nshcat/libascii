#include <stdexcept>
#include <ut/throwf.hxx>
#include <GLXW/glxw.h>
#include <screen.hxx>


screen_manager::screen_manager(dimension_type p_screenSize)
	:	m_ScreenDims{p_screenSize},
		m_Data(p_screenSize.x * p_screenSize.y)	
{
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_GPUTexture);
	glGenBuffers(1, &m_GPUBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, m_GPUBuffer);
	glBufferData(GL_TEXTURE_BUFFER, m_Data.size()*sizeof(cell), static_cast<GLvoid*>(m_Data.data()), GL_DYNAMIC_DRAW);
	glBindTexture(GL_TEXTURE_BUFFER, m_GPUTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, m_GPUBuffer);
}


void screen_manager::sync()
{
	if(m_Dirty)
	{
		glBindBuffer(GL_TEXTURE_BUFFER, m_GPUBuffer);
		
		glBufferData(GL_TEXTURE_BUFFER, m_Data.size()*sizeof(cell), static_cast<GLvoid*>(m_Data.data()), GL_DYNAMIC_DRAW);
		
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	
		m_Dirty = false;
	}
}

auto screen_manager::calc_index(position_type p_pos) const
	-> index_type
{
	return ((m_ScreenDims.x * p_pos.y) + p_pos.x);
}

void screen_manager::set_dirty()
{
	m_Dirty = true;
}

cell& screen_manager::get_cell(index_type p_idx)
{
	return m_Data[p_idx];
}

const cell& screen_manager::get_cell(index_type p_idx) const
{
	return m_Data[p_idx];
}

void screen_manager::clear_cell(index_type p_idx)
{
	get_cell(p_idx) = cell{ };
}

bool screen_manager::check_position(position_type p_pos) const
{
	return (p_pos.x < m_ScreenDims.x && p_pos.y < m_ScreenDims.y);
}

void screen_manager::clear_screen()
{
	for(auto& t_entry: m_Data)
		t_entry = cell{ };
		
	set_dirty();
}

void screen_manager::clear_cell(position_type p_pos)
{
	if(!check_position(p_pos))
		ut::throwf<::std::runtime_error>("screen_manager::clear_cell: Position out of bounds: (%u, %u)", p_pos.x, p_pos.y);

	clear_cell(calc_index(p_pos));
	set_dirty();
}

cell& screen_manager::modify_cell(position_type p_pos)
{
	if(!check_position(p_pos))
		ut::throwf<::std::runtime_error>("screen_manager::modify_cell: Position out of bounds: (%u, %u)", p_pos.x, p_pos.y);

	set_dirty();
	
	return get_cell(calc_index(p_pos));
}

const cell& screen_manager::read_cell(position_type p_pos) const
{
	if(!check_position(p_pos))
		ut::throwf<::std::runtime_error>("screen_manager::read_cell: Position out of bounds: (%u, %u)", p_pos.x, p_pos.y);
	
	return get_cell(calc_index(p_pos));
}

void screen_manager::set_cell(position_type p_pos, const cell& p_cell)
{
	if(!check_position(p_pos))
		ut::throwf<::std::runtime_error>("screen_manager::set_cell: Position out of bounds: (%u, %u)", p_pos.x, p_pos.y);
	
	m_Data[calc_index(p_pos)] = p_cell;
	
	set_dirty();
}



