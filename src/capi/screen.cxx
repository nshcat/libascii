#include <capi/screen.h>
#include <global_state.hxx>

extern "C"
{
	void screen_get_dimensions(uvec2_t* p_out)
	{
		const auto& t_dims = global_state<render_manager>().screen().screen_size();
		
		p_out->x = t_dims.x;
		p_out->y = t_dims.y;
	}
	
	void screen_set_tile(uvec2_t* p_pos, uvec3_t* p_front, uvec3_t* p_back, uint8_t p_glyph)
	{
		auto& t_cell = global_state<render_manager>().screen().modify_cell(*reinterpret_cast<glm::uvec2*>(p_pos));
		
		t_cell.set_fg(*reinterpret_cast<glm::uvec3*>(p_front));
		t_cell.set_bg(*reinterpret_cast<glm::uvec3*>(p_back));
		t_cell.set_glyph(p_glyph);
	}
	
	void screen_clear_tile(uvec2_t* p_pos)
	{
		global_state<render_manager>().screen().clear_cell(*reinterpret_cast<glm::uvec2*>(p_pos));
	}
	
	void screen_apply_commands(command_t* p_cmdbuf, int p_count)
	{
		auto& t_scr = global_state<render_manager>().screen();
	
		for(int i = 0; i < p_count; ++i)
		{
			auto& t_cmd = p_cmdbuf[i];
			
			glm::uvec2 t_pos{ t_cmd.position.x, t_cmd.position.y };
			
			auto& t_cell = t_scr.modify_cell(t_pos);
			
			switch(t_cmd.type)
			{
				case CMD_SET_GLYPH:
				{
					t_cell.set_glyph((::std::uint8_t)t_cmd.value);
					break;
				}
				case CMD_SET_FG:
				{
					glm::uvec3 t_clr{
						t_cmd.color.r,
						t_cmd.color.g,
						t_cmd.color.b
					};
					
					t_cell.set_fg(t_clr);
					break;
				}
				case CMD_SET_BG:
				{
					glm::uvec3 t_clr{
						t_cmd.color.r,
						t_cmd.color.g,
						t_cmd.color.b
					};
				
					t_cell.set_bg(t_clr);
					break;
				}
				case CMD_SET_DEPTH:
				{
					t_cell.set_depth((::std::uint8_t)t_cmd.value);
					break;
				}
				case CMD_CLEAR_TILE:
				{
					t_scr.clear_cell(t_pos);
					break;
				}
				case CMD_SET_LIGHT_MODE:
				{
					t_cell.set_light_mode(static_cast<light_mode>(t_cmd.value));
					break;
				}
				case CMD_SET_GUI_MODE:
				{
					t_cell.set_gui_mode(t_cmd.value);
					break;
				}
				default:
				{
					LOG_F_TAG("libascii") << "apply_commands: Invalid command type \"" << t_cmd.type << "\"";
					break;
				}
			}
		}
	}
	
	void screen_set_light_mode(uvec2_t* p_pos, int p_mode)
	{
		glm::uvec2 t_pos{ p_pos->x, p_pos->y };
		global_state<render_manager>().screen().modify_cell(t_pos).set_light_mode(ut::enum_cast<light_mode>(p_mode));
	}
	
	void screen_set_gui_mode(uvec2_t* p_pos, bool_t p_flag)
	{
		glm::uvec2 t_pos{ p_pos->x, p_pos->y };
		auto t_flag = static_cast<bool>(p_flag);
	
		global_state<render_manager>().screen().modify_cell(t_pos).set_gui_mode(t_flag);
	}
	
	void screen_clear()
	{
		auto& t_screen = global_state<render_manager>().screen();
		t_screen.clear();
		
		const auto t_screenDims = t_screen.screen_size();
		
		for(int iy = 0; iy < t_screenDims.y; ++iy)
		{
			for(int ix = 0; ix < t_screenDims.x; ++ix)
			{
				t_screen.modify_cell({ix, iy}).set_gui_mode(true);
			}
		}
		
	}
	
	void screen_set_depth(uvec2_t* p_pos, uint8_t p_depth)
	{
		glm::uvec2 t_pos{ p_pos->x, p_pos->y };
	
		global_state<render_manager>().screen().modify_cell(t_pos).set_depth(p_depth);
	}
}
