// TODO nuklear gui class
// TODO fps limiter class (see stackoverflow links in telegram)
// TODO maybe global type definitions? (color, dimension, point, glyph...)

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <utility>
#include <random>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <ut/cast.hxx>
#include <minitrace.h>
#include <program.hxx>
#include <shader.hxx>
#include <texture_set.hxx>
#include <uniform.hxx>
#include <weighted_distribution.hxx>
#include <global_state.hxx>
#include <renderer.hxx>
#include <lighting.hxx>
#include <screen.hxx>
#include <actions.hxx>
#include <shapes.hxx>
#include <frame_guard.hxx>
#include <palette.hxx>
#include <build_settings.hxx>
#include <path_manager.hxx>



#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include <nuklear.h>
#include <nuklear_glfw_gl3.h>


bool clear_ = true;
bool pressed_ = false;
bool clear__ = true;
bool pressed__ = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
    	clear_ = pressed_ = true;
    }
    
    if (key == GLFW_KEY_E && action == GLFW_REPEAT)
    {
    	clear_ = false;
    	pressed_ = true;
    }
    
    if (key == GLFW_KEY_E && action == GLFW_RELEASE)
    {
    	clear_ = true;
    	pressed_ = false;
    }
   
    if ((mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS)
    {
    	clear__ = pressed__ = true;
    }
    
    if ((mods & GLFW_MOD_SHIFT) && action == GLFW_REPEAT)
    {
    	clear__ = false;
    	pressed__ = true;
    }
    
    if ((mods & GLFW_MOD_SHIFT) && action == GLFW_RELEASE)
    {
    	clear__ = true;
    	pressed__ = false;
    }
}

int main()
{
	global_state().initialize();
	
	::std::cout << "User data path: " << global_state().path_manager().user_path() << ::std::endl;
	::std::cout << "Game data path: " << global_state().path_manager().data_path() << ::std::endl;
	::std::cout << "Config file path: " << global_state().path_manager().config_path() << ::std::endl << ::std::endl;

	::std::cout << *global_state().configuration().get<int>("test") << ::std::endl;

	nk_context* t_nkctx;
	nk_color t_nkbg;

	try
	{
		palette t_palette{ "assets/palettes/c64.json" };
	
		//global_state().context().initialize();
		
		// Load texture
		texture_set t_texSet{
			shadow_texture("assets/textures/default/shadows.png"),
			text_texture("assets/textures/default/text.png"),
			graphics_texture("assets/textures/default/graphics.png")
		};
		
		render_manager t_renderer{
			global_state().context(),
			{ 60, 30 },
			t_texSet
		};
		
		
		t_nkctx = nk_glfw3_init(global_state().context().handle(), NK_GLFW3_INSTALL_CALLBACKS);
		nk_font_atlas* atlas;
    	nk_glfw3_font_stash_begin(&atlas);
    	nk_font* t_fontMenlo = nk_font_atlas_add_from_file(atlas, "assets/fonts/menlo.ttf", 17, 0);
		nk_glfw3_font_stash_end();
		nk_style_set_font(t_nkctx, &t_fontMenlo->handle);	
		t_nkbg = nk_rgb(28,48,62);
		
		
		
		std::random_device rd;
		std::mt19937 t_gen(rd());
    	std::uniform_int_distribution<unsigned> t_distrib(0, 16);
    	std::uniform_real_distribution<float> t_intensityDistrib(0.4f, 1.0f); 
		
		/*weighted_distribution<glm::uvec3> t_groundClr{
			{ { 84, 84, 84 }, 0.3f },		
			{ { 94, 94, 94 }, 0.3f },		
			{ { 56, 56, 56 }, 0.2f },		
			{ { 75, 75, 75 }, 0.2f }
		};*/
		
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
		auto& t_screenManager = t_renderer.screen();
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
	
		
		
				
		//===----------------------------------------------------------------------===//
		// Lighting
		//	
		
		// Create light manager
		auto& t_lightManager = t_renderer.light_manager();	
		
		// Create one light
		light t_light{
			glm::ivec2{10.f, 7.f},
			0.7f,
			gpu_bool(true),
			glm::vec4{ 1.f, .647f, 0.f, 1.f },
			glm::vec3{ 0.f },
			1.5f
		};
		
		// Register it and save handle
		const auto t_lightHandle = t_lightManager.create_light(t_light);
		
		// Setup ambient light
		nk_color t_ambient;
		
		{
			const auto& t_ref = t_lightManager.modify_state().m_AmbientLight;
			t_ambient = nk_color{
				(nk_byte)(t_ref.r * 255.f),
				(nk_byte)(t_ref.g * 255.f),
				(nk_byte)(t_ref.b * 255.f),
				(nk_byte)(t_ref.a * 255.f)
			};
		}
		
			
		//===----------------------------------------------------------------------===//
		
		
				
		::std::size_t inputCounter = 0;
		const ::std::size_t inputPeriod = 4;

		::std::size_t animCounter = 0;
		const ::std::size_t animPeriod = 6;

		auto& t_context = global_state().context();

		while (!t_context.should_close())
		{	
			t_context.pump_events();
			
			nk_glfw3_new_frame();		
			if (nk_begin(t_nkctx, "Debug", nk_rect(700, 100, 330, 350),
            	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            	NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        	{
        		nk_layout_row_static(t_nkctx, 30, 100, 2);
        		
        		if(nk_button_label(t_nkctx, "Lighting"))
        		{
        			auto& t_ref = t_lightManager.modify_state();
					t_ref.m_UseLighting = !t_ref.m_UseLighting;
        		}
        		
        		if(nk_button_label(t_nkctx, "Dynamic"))
        		{
        			auto& t_ref = t_lightManager.modify_state();
					t_ref.m_UseDynamic = !t_ref.m_UseDynamic;
        		}
        		    		
        		nk_layout_row_static(t_nkctx, 30, 200, 1);
        		nk_label(t_nkctx, "", NK_TEXT_ALIGN_LEFT);
        		
        		nk_layout_row_static(t_nkctx, 30, 200, 1);
        		
        		//bool t_pressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
        		bool t_pressed = pressed_;
        		
        		if(clear_)
        		{
        			pressed_ = false;
        		}
        		
        		bool t_pressed2 = pressed__;
        		
        		if(clear__)
        		{
        			pressed__ = false;
        		}
        		
        		nk_menu_item_label(t_nkctx, (t_pressed ? "E pressed" : "E not pressed"), NK_TEXT_LEFT);
        		nk_menu_item_label(t_nkctx, (t_pressed2 ? "Shift pressed" : "Shift not pressed"), NK_TEXT_LEFT);
        		
        		nk_layout_row_static(t_nkctx, 30, 200, 1);
        		nk_label(t_nkctx, "Ambient light:", NK_TEXT_ALIGN_LEFT);
        		
            	if (nk_combo_begin_color(t_nkctx, t_ambient, nk_vec2(nk_widget_width(t_nkctx),400))) {
		        	nk_layout_row_dynamic(t_nkctx, 120, 1);
		            t_ambient = nk_color_picker(t_nkctx, t_ambient, NK_RGBA);
		            nk_layout_row_dynamic(t_nkctx, 25, 1);
		            t_ambient.r = (nk_byte)nk_propertyi(t_nkctx, "#R:", 0, t_ambient.r, 255, 1,1);
		            t_ambient.g = (nk_byte)nk_propertyi(t_nkctx, "#G:", 0, t_ambient.g, 255, 1,1);
		            t_ambient.b = (nk_byte)nk_propertyi(t_nkctx, "#B:", 0, t_ambient.b, 255, 1,1);
		            t_ambient.a = (nk_byte)nk_propertyi(t_nkctx, "#A:", 0, t_ambient.a, 255, 1,1);
		            nk_combo_end(t_nkctx);
            	}
            	
            	t_lightManager.modify_state().m_AmbientLight =
            		glm::vec4{float(t_ambient.r)/255.f, float(t_ambient.g)/255.f, float(t_ambient.b)/255.f, float(t_ambient.a)/255.f};
            		
        		
        		float t_radius = t_light.m_Radius;
        		nk_layout_row_static(t_nkctx, 30, 200, 1);
        		nk_property_float(t_nkctx, "Radius:", 0.f, &t_radius, 10.f, 0.1f, 0.1f);
        		
        		
        		if(t_radius != t_light.m_Radius)
        		{
        			t_lightManager.modify_light(t_lightHandle).m_Radius = t_radius;
        			t_light.m_Radius = t_radius;
        		}
        	}
				
			nk_end(t_nkctx);
			
		
			if(animCounter >= animPeriod)
			{
				animCounter = 0;
			
				const float t_intensityMod = t_intensityDistrib(t_gen);
				
				// We can use the intensity of the t_light object here, since
				// that will never change
				t_lightManager.modify_light(t_lightHandle).m_Intensity = 
					t_light.m_Intensity * t_intensityMod;		
				
			}
			else ++animCounter;
		

			//frame_guard t_frame{ };
			t_context.begin_frame();
			{		
				t_renderer.render();
				nk_glfw3_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
			}
			t_context.end_frame();
		}
		
		nk_glfw3_shutdown();
	}
	catch(const gl::shader_exception& p_ex)
	{
		std::cout << "An exception was thrown: " << p_ex.what() << '\n' << p_ex.log() << std::endl;
	}
}
