#include <cstdint>
#include <random>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <ut/cast.hxx>

#include <commandline.hxx>
#include <engine.hxx>
#include <weighted_distribution.hxx>
#include <log_manager.hxx>
#include <application_layer/config/config_entry.hxx>
#include <application_layer/config/config_scheme.hxx>
#include <global_state.hxx>
#include <actions.hxx>
#include <shapes.hxx>


enum class config_value_type
{
	t_float = 0,
	t_double,
	t_int,
	t_uint,
	t_string,
	t_bool
};

enum class command_type
{
	set_glyph,
	set_bg,
	set_fg,
	set_depth,
	clear_tile
};

struct command
{
	command_type m_Type;
	glm::uvec2 m_Position;
	
	union
	{
		glm::uvec3 m_Color;
		::std::uint8_t m_Value;
	};
};

struct game_info_t
{
	const char* m_Name;
	const char* m_Version;
	const char* m_Description;
};

namespace internal
{
	auto level_color(lg::severity_level p_lvl)
		-> ut::console_color
	{
		switch(p_lvl)
		{
			case lg::severity_level::fatal:
				return ut::console_color::bright_red;
				break;
			case lg::severity_level::error:
				return ut::console_color::bright_red;
				break;
			case lg::severity_level::warning:
				return ut::console_color::bright_yellow;
				break;
			case lg::severity_level::info:
				return ut::console_color::bright_white;
				break;	
			case lg::severity_level::debug:
				return ut::console_color::bright_cyan;
				break;
			default:
				LOG_F_TAG("libascii") << "Invalid log message severity supplied: " << ut::enum_cast(p_lvl);
				::std::exit(EXIT_FAILURE);	
		}
	}
	
	template< typename T >
	void* alloc_config_entry(ut::string_view p_path)
	{
		const auto t_val = global_state<configuration>().get<T>({p_path.to_string()});
		
		if(!t_val)
		{
			LOG_F_TAG("libascii") << "Requested configuration entry does not exist: " << p_path;
			::std::exit(EXIT_FAILURE);
		}
		
		auto t_ptr = ::std::make_unique<T>(*t_val);
		return t_ptr.release();
	}
	
	template< >
	void* alloc_config_entry<::std::string>(ut::string_view p_path)
	{
		const auto t_val = global_state<configuration>().get<::std::string>({p_path.to_string()});
		
		if(!t_val)
		{
			LOG_F_TAG("libascii") << "Requested configuration entry does not exist: " << p_path;
			::std::exit(EXIT_FAILURE);
		}
		
		char* t_str = new char[t_val->size() + 1];
		
		::std::copy(t_val->begin(), t_val->end(), t_str);
		t_str[t_val->size()] = '\0';
		
		return t_str;
	}
}


extern "C"
{
	void free_memory(void* ptr)
	{
		free(ptr);
	}
	
	void engine_initialize(game_info_t* info, ::std::int64_t argc, const char** argv)
	{
		game_info t_info{
			{ info->m_Name },
			{ info->m_Version },
			{ info->m_Description }
		};
	
		engine::initialize(t_info, static_cast<int>(argc), argv);
	}
	
	bool render_context_should_close()
	{
		return global_state<render_context>().should_close();
	}
	
	void render_context_begin_frame()
	{
		global_state<render_context>().begin_frame();
	}
	
	void render_context_end_frame()
	{
		global_state<render_context>().end_frame();
	}
	
	void logger_post_message(int lvl, const char* tag, const char* msg)
	{
		const auto t_lvl = ut::enum_cast<lg::severity_level>(lvl);
		const auto t_clr = internal::level_color(t_lvl);	
		
		if(tag == nullptr || ::std::strlen(tag) == 0)
			LOGGER() += lg::log_entry("libascii", 0) << t_lvl << t_clr << msg;
		else
			LOGGER() += lg::log_entry("libascii", 0) << t_lvl << t_clr << lg::tag(::std::string{tag}) << msg;
	}
	
	void debug_create_test_scene()
	{
		auto t_palette = global_state<asset_manager>().load_asset<palette>("c64");
	
		std::random_device rd;
		std::mt19937 t_gen(rd());
    	std::uniform_int_distribution<unsigned> t_distrib(0, 16);
    	std::uniform_real_distribution<float> t_intensityDistrib(0.4f, 1.0f); 
		
		weighted_distribution<glm::uvec3> t_groundClr{
			{ { 0, 102, 43 }, 0.2f },
			{ { 68, 102, 41 }, 0.2f },
			{ { 0, 62, 26 }, 0.2f },
			{ { 107, 107, 54 }, 0.15f },		
			{ { 51, 77, 31 }, 0.1f },		
			{ { 85, 128, 51 }, 0.06f },
			{ { 0, 92, 38 }, 0.06f },
			{ { 94, 94, 94 }, 0.03f }
		};
	
		//===----------------------------------------------------------------------===//
		// Screen
		//
		auto& t_screenManager = global_state<render_manager>().screen();
		t_screenManager.modify(draw_border<thin_border_style>({0, 0}, {21, 21}, set(glyph_set::graphics)));
		t_screenManager.modify(draw_string({1, 0}, "Fog"));
		
		for(::std::size_t t_ix = 1; t_ix < 20; t_ix += 2)
		{
			t_screenManager.modify(area({t_ix, 1}, {t_ix+1, 20}),
				sequence(
					sample_background(t_groundClr, rd),
					set_depth(t_ix/2)
				)
			);
			
			if(t_ix >= 3)
				t_screenManager.modify(area({t_ix, 1}, {t_ix, 20}),
					set_shadows(drop_shadow::west)
				);
		}
		
		t_screenManager.modify(draw_border<thin_border_style>({23, 0}, {32, 3}, set(glyph_set::graphics)));
		t_screenManager.modify(draw_string({24, 0}, "Palette"));
		
		for(::std::size_t t_ix = 0; t_ix < 8; ++t_ix)
		{
			t_screenManager.modify(point({24+t_ix, 1}), draw(background(t_palette.lookup(ut::enum_cast<color>(t_ix)))));
			t_screenManager.modify(point({24+t_ix, 2}), draw(background(t_palette.lookup(ut::enum_cast<color>(t_ix+8)))));
		}
		
		//===----------------------------------------------------------------------===//
	}
	
	void renderer_render()
	{
		global_state<render_manager>().render();
	}
	
	void* configuration_get(config_value_type type, const char* path)
	{
		switch(type)
		{
			case config_value_type::t_float:
				return internal::alloc_config_entry<float>({path});
			case config_value_type::t_double:
				return internal::alloc_config_entry<double>({path});
			case config_value_type::t_int:
				return internal::alloc_config_entry<int>({path});
			case config_value_type::t_uint:
				return internal::alloc_config_entry<unsigned>({path});
			case config_value_type::t_bool:
				return internal::alloc_config_entry<bool>({path});
			case config_value_type::t_string:
				return internal::alloc_config_entry<::std::string>({path});
			default:
				LOG_F_TAG("libascii") << "Invalid configuration entry type: " << ut::enum_cast(type);
				::std::exit(EXIT_FAILURE);
				
		}
	}
	
	const char* path_get_data_path()
	{
		const auto p = global_state<path_manager>().data_path().string();
		char* str = new char[p.size()+1];
		::std::copy(p.begin(), p.end(), str);
		str[p.size()] = '\0';
		
		return str;
	}
	
	const char* path_get_user_path()
	{
		const auto p = global_state<path_manager>().user_path().string();
		char* str = new char[p.size()+1];
		::std::copy(p.begin(), p.end(), str);
		str[p.size()] = '\0';
		
		return str;
	}
	
	void screen_get_dimensions(glm::uvec2* out)
	{
		const auto& t_dims = global_state<render_manager>().screen().screen_size();
		
		out->x = t_dims.x;
		out->y = t_dims.y;
	}
	
	void screen_set_tile(glm::uvec2* pos, glm::uvec3* front, glm::uvec3* back, ::std::uint8_t glyph)
	{
		auto& t_cell = global_state<render_manager>().screen().modify_cell(*pos);
		
		t_cell.set_fg(*front);
		t_cell.set_bg(*back);
		t_cell.set_glyph(glyph);
	}
	
	void screen_clear_tile(glm::uvec2* pos)
	{
		global_state<render_manager>().screen().clear_cell(*pos);
	}
	
	void screen_apply_commands(command* p_cmdbuf, int p_count)
	{
		auto& t_scr = global_state<render_manager>().screen();
	
		for(int i = 0; i < p_count; ++i)
		{
			auto& t_cmd = p_cmdbuf[i];
			
			auto& t_cell = t_scr.modify_cell(t_cmd.m_Position);
			
			switch(t_cmd.m_Type)
			{
				case command_type::set_glyph:
				{
					t_cell.set_glyph(t_cmd.m_Value);
					break;
				}
				case command_type::set_fg:
				{
					t_cell.set_fg(t_cmd.m_Color);
					break;
				}
				case command_type::set_bg:
				{
					t_cell.set_bg(t_cmd.m_Color);
					break;
				}
				case command_type::set_depth:
				{
					t_cell.set_depth(t_cmd.m_Value);
					break;
				}
				case command_type::clear_tile:
				{
					t_scr.clear_cell(t_cmd.m_Position);
					break;
				}
				default:
				{
					LOG_F_TAG("libascii") << "apply_commands: Invalid command type \"" << ut::enum_cast(t_cmd.m_Type) << "\"";
					break;
				}
			}
		}
	}
	
	void screen_clear()
	{
		global_state<render_manager>().screen().clear();
	}
	
	void screen_set_depth(glm::uvec2* pos, ::std::uint8_t depth)
	{
		global_state<render_manager>().screen().modify_cell(*pos).set_depth(depth);
	}
	
	void input_begin()
	{
		global_state<input_manager>().begin_input();
	}
	
	void input_end()
	{
		global_state<input_manager>().end_input();
	}
	
	bool input_has_key(int key)
	{
		return global_state<input_manager>().has_key(key);
	}
}


