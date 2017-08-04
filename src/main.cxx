#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <utility>
#include <random>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstdint>
#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <program.hxx>
#include <shader.hxx>
#include <texture.hxx>
#include <uniform.hxx>
#include <shadow_texture.hxx>
#include <weighted_collection.hxx>
#include <lighting.hxx>

constexpr ::std::uint32_t SHADOW_N = 0x1 << 8;
constexpr ::std::uint32_t SHADOW_W = 0x1 << 9;
constexpr ::std::uint32_t SHADOW_S = 0x1 << 10;
constexpr ::std::uint32_t SHADOW_E = 0x1 << 11;

constexpr ::std::uint32_t SHADOW_TL = 0x1 << 12;
constexpr ::std::uint32_t SHADOW_TR = 0x1 << 13;
constexpr ::std::uint32_t SHADOW_BL = 0x1 << 14;
constexpr ::std::uint32_t SHADOW_BR = 0x1 << 15;

constexpr ::std::uint32_t FOG_MASK = 0xFF;

constexpr ::std::uint32_t LIGHT_SHIFT = 16;
constexpr ::std::uint32_t LIGHT_NONE = 0;
constexpr ::std::uint32_t LIGHT_DIM = 1;
constexpr ::std::uint32_t LIGHT_FULL = 2;


void error_callback(int error, const char* description)
{
	std::cerr << "glfw error: " << description << std::endl;
	std::exit(EXIT_FAILURE);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
	bool useLighting{true};
	bool lightModeDebug{false};
	bool useDynamicLighting{true};
	GLFWwindow* window;

	try
	{
		GLuint vertex_buffer;
		GLint mvp_location, vpos_location, vcol_location;
	
		if(!glfwInit())
		{
		    std::cerr << "failed to init GLFW" << std::endl;
		    std::exit(EXIT_FAILURE);
		}
		
		glfwSetErrorCallback(error_callback);
		
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		if(!(window = glfwCreateWindow(100, 100, "gl_app", 0, 0)))
		{
		    std::cerr << "failed to open window" << std::endl;
		    glfwTerminate();
		    std::exit(EXIT_FAILURE);
		}
		
		glfwSetKeyCallback(window, key_callback);
		
		glfwMakeContextCurrent(window);
		
		if(glxwInit())
		{
		    std::cerr << "failed to init GLXW" << std::endl;
		    glfwDestroyWindow(window);
		    glfwTerminate();
		    std::exit(EXIT_FAILURE);
		}
		
		glfwSwapInterval(1);

		const auto t_verStr = glGetString(GL_VERSION);
		std::cout << "OpenGL version: " << t_verStr << std::endl;
		
		// Load texture
		glEnable(GL_TEXTURE_2D);
		texture t_tex{ "assets/CLA.png" };
		//texture t_tex{ "assets/tex4.png" };
		shadow_texture t_shadow{ "assets/shadows.png" };
		
		// Resize window
		const glm::ivec2 t_glyphDim = t_tex.glyph_size();
		const glm::ivec2 t_glyphCount = /*{ 20, 20 };//*/ { 60, 30 };
		
		const auto t_width = t_glyphDim.x * t_glyphCount.x;
		const auto t_height = t_glyphDim.y * t_glyphCount.y;
		
		glfwSetWindowSize(window, t_width, t_height);
		
		
		// Glyph data set
		::std::vector<glm::uvec4> t_Data;
		t_Data.resize(t_glyphCount.x * t_glyphCount.y * 2);
		
		std::random_device rd;
		std::mt19937 t_gen(rd());
    	std::uniform_int_distribution<unsigned> t_distrib(0, 16);
    	std::uniform_real_distribution<float> t_intensityDistrib(0.4f, 1.0f); 
		
		weighted_collection<glm::uvec3> t_groundClr{
			{ { 84, 84, 84 }, 0.3f },		
			{ { 94, 94, 94 }, 0.3f },		
			{ { 56, 56, 56 }, 0.2f },		
			{ { 75, 75, 75 }, 0.2f }
		};
						
		t_groundClr.shuffle(t_gen);
		
		
		for(::std::size_t ix = 0; ix < t_Data.size(); ix+=2)
		{		
			t_Data[ix] = glm::uvec4{0, 0, 0, 0};
			t_Data[ix+1] = glm::uvec4{0, 0, 0, 0};
		}
		
		
		const auto t_pos = [&t_glyphCount](int x, int y) -> int
		{
			return ((t_glyphCount.x * y) + x) * 2;
		};
		
		for(::std::size_t iy = 2; iy < 30-2; ++iy)
		{
			if(iy == 2 || iy == 30-3)
			{
				for(::std::size_t ix = 2; ix < 50-2; ++ix)
				{
					t_Data[t_pos(ix, iy)] = glm::uvec4{187, 187, 187, 61};
					t_Data[t_pos(ix, iy)+1] = glm::uvec4{187, 187, 187, 0};
					
					t_Data[t_pos(ix, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
				}
			}
			else
			{
				t_Data[t_pos(2, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(2, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(2, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
				
				t_Data[t_pos(50-3, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(50-3, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(50-3, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
			}
		}		
		
		for(::std::size_t iy = 3; iy < 30-3; ++iy)
		{
			for(::std::size_t ix = 3; ix < 50-3; ++ix)
			{
				auto t_clr = t_groundClr(t_gen);
				t_Data[t_pos(ix, iy)] = glm::uvec4{0, 0, 0, 0};
				t_Data[t_pos(ix, iy)+1] = glm::uvec4{t_clr.r, t_clr.g, t_clr.b, 0};
				t_Data[t_pos(ix, iy)+1].a |= (LIGHT_FULL << LIGHT_SHIFT);
			}
		}
		
		for(::std::size_t iy = 6; iy < 20-6; ++iy)
		{
			if(iy == 6 || iy == 20-7)
			{
				for(::std::size_t ix = 6; ix < 20-6; ++ix)
				{
					if((ix % 2) != 0)
						continue;
				
					t_Data[t_pos(ix, iy)] = glm::uvec4{187, 187, 187, 61};
					t_Data[t_pos(ix, iy)+1] = glm::uvec4{187, 187, 187, 0};
					
					t_Data[t_pos(ix, iy)+1].a = (LIGHT_DIM << LIGHT_SHIFT);
				}
			}
			else
			{
				t_Data[t_pos(6, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(6, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(6, iy)+1].a = (LIGHT_DIM << LIGHT_SHIFT);
				
				t_Data[t_pos(20-7, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(20-7, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(20-7, iy)+1].a = (LIGHT_DIM << LIGHT_SHIFT);
			}
		}
		
		
		// Create buffer
		glActiveTexture(GL_TEXTURE1);
		GLuint t_buftex;
		glGenTextures(1, &t_buftex);
		GLuint t_buf;
		glGenBuffers(1, &t_buf);
		glBindBuffer(GL_TEXTURE_BUFFER, t_buf);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::uvec4)*t_Data.size(), (GLvoid*)t_Data.data(), GL_DYNAMIC_DRAW);	
		glBindTexture(GL_TEXTURE_BUFFER, t_buftex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, t_buf);
			
		
		// Empty vertex buffer
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		
		
		
		gl::program program{
			gl::vertex_shader{ gl::from_file, "assets/ascii.vs.glsl" },
			gl::fragment_shader{ gl::from_file, "assets/ascii.fs.glsl" },
		};
	
		program.use();
		
	
	
		
		//===----------------------------------------------------------------------===//
		// Uniforms
		//
		
		// Set uniforms		
		gl::set_uniform(program, "fog_color", glm::vec4{ 0.1f, 0.1f, 0.3f, 1.f });
		gl::set_uniform(program, "fog_density", 5.f);
		gl::set_uniform(program, "projection_mat", glm::ortho(0.f, (float)t_width, (float)t_height, 0.f, -1.f, 1.f));		
		gl::set_uniform(program, "sheet_dimensions", glm::ivec2{ texture::sheet_width, texture::sheet_height });
		gl::set_uniform(program, "glyph_dimensions", t_glyphDim);
		gl::set_uniform(program, "glyph_count", t_glyphCount);
		
		// Samplers are just integers
		gl::set_uniform(program, "sheet_texture", 0);
		gl::set_uniform(program, "input_buffer", 1);
		gl::set_uniform(program, "shadow_texture", 2);
		//===----------------------------------------------------------------------===/
		
		
		
				
		//===----------------------------------------------------------------------===//
		// Lighting
		//	
		
		// Create light manager
		light_manager t_lightManager{ };	
		
		// Create one light
		light t_light{
			glm::ivec2{10.f, 7.f},
			0.7f,
			gpu_bool(true),
			glm::vec4{ 1.f, .647f, 0.f, 1.f },
			glm::vec3{ 0.f },
			3.f
		};
		
		// Register it and save handle
		const auto t_lightHandle = t_lightManager.create_light(t_light);	
		//===----------------------------------------------------------------------===//




		::std::size_t inputCounter = 0;
		const ::std::size_t inputPeriod = 4;

		::std::size_t animCounter = 0;
		const ::std::size_t animPeriod = 6;

		while (!glfwWindowShouldClose(window))
		{
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
		
			if(inputCounter >= inputPeriod)
			{			
				inputCounter = 0;
				
				if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				{			
					auto t_ref = t_lightManager.modify_state();
					t_ref.m_UseLighting = !t_ref.m_UseLighting;
				}	
				
				if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					auto t_ref = t_lightManager.modify_state();
					t_ref.m_UseDynamic = !t_ref.m_UseDynamic;
				}			
				
				if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				{
					t_light.m_Position.y = glm::max(t_light.m_Position.y - 1.f, 3.f); 
					t_lightManager.modify_light(t_lightHandle).m_Position = t_light.m_Position;
				}
				
				if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				{
					t_light.m_Position.y = glm::min(t_light.m_Position.y + 1.f, 30.f-4.f); 
					t_lightManager.modify_light(t_lightHandle).m_Position = t_light.m_Position;
				}
				
				if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				{				
					t_light.m_Position.x = glm::max(t_light.m_Position.x - 1.f, 3.f); 
					t_lightManager.modify_light(t_lightHandle).m_Position = t_light.m_Position;
				}
				
				if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				{				
					t_light.m_Position.x = glm::min(t_light.m_Position.x + 1.f, 50.f-4.f); 
					t_lightManager.modify_light(t_lightHandle).m_Position = t_light.m_Position;
				}
				
			}
			else ++inputCounter;
			
			// Sync local light data with gpu buffer
			t_lightManager.sync();		
		
		    float ratio;
		    int width, height;

		    glfwGetFramebufferSize(window, &width, &height);
		    ratio = width / (float) height;

		    glViewport(0, 0, width, height);
		    glClear(GL_COLOR_BUFFER_BIT);

		   	program.use();
		    
			// We need to render 6 vertices per glyph.
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, t_glyphCount.x * t_glyphCount.y);

		    glfwSwapBuffers(window);
		    glfwPollEvents();
		}
		
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	catch(const gl::shader_exception& p_ex)
	{
		std::cout << "An exception was thrown: " << p_ex.what() << '\n' << p_ex.log() << std::endl;
	}
}
