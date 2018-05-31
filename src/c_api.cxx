#include <cstdint>
#include <random>
#include <iostream>
#include <cstring>
#include <ut/cast.hxx>

#include <commandline.hxx>
#include <weighted_distribution.hxx>
#include <log_manager.hxx>
#include <application_layer/config/config_entry.hxx>
#include <application_layer/config/config_scheme.hxx>
#include <global_state.hxx>
#include <actions.hxx>
#include <shapes.hxx>


extern "C"
{
	void engine_initialize(::std::int64_t argc, const char** argv)
	{
		// Populate command line handler
		g_clHandler.read(static_cast<int>(argc), argv);
		
		// Init global state
		global_state().initialize();	
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
		ut::console_color t_clr{ };
		
		switch(t_lvl)
		{
			case lg::severity_level::fatal:
				t_clr = ut::console_color::bright_red;
				break;
			case lg::severity_level::error:
				t_clr = ut::console_color::bright_red;
				break;
			case lg::severity_level::warning:
				t_clr = ut::console_color::bright_yellow;
				break;
			case lg::severity_level::info:
				t_clr = ut::console_color::bright_white;
				break;	
			case lg::severity_level::debug:
				t_clr = ut::console_color::bright_cyan;
				break;
			default:
				LOG_F_TAG("libascii") << "Invalid log message severity supplied: " << lvl;
				::std::exit(EXIT_FAILURE);	
		}
		
		
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
}


