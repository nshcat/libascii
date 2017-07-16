#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <utility>
#include <random>
#include <cstdlib>

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
#include <shadow_texture.hxx>
#include <weighted_collection.hxx>

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
		const glm::ivec2 t_glyphCount = /*{ 20, 20 };//*/ { 20, 20 };
		
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
		
		for(::std::size_t iy = 5; iy < 20-5; ++iy)
		{
			if(iy == 5 || iy == 20-6)
			{
				for(::std::size_t ix = 5; ix < 20-5; ++ix)
				{
					t_Data[t_pos(ix, iy)] = glm::uvec4{187, 187, 187, 61};
					t_Data[t_pos(ix, iy)+1] = glm::uvec4{187, 187, 187, 0};
					
					t_Data[t_pos(ix, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
				}
			}
			else
			{
				t_Data[t_pos(5, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(5, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(5, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
				
				t_Data[t_pos(20-6, iy)] = glm::uvec4{187, 187, 187, 61};
				t_Data[t_pos(20-6, iy)+1] = glm::uvec4{187, 187, 187, 0};
				t_Data[t_pos(20-6, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
			}
		}
		
		for(::std::size_t iy = 6; iy < 20-6; ++iy)
		{
			for(::std::size_t ix = 6; ix < 20-6; ++ix)
			{
				auto t_clr = t_groundClr(t_gen);
				t_Data[t_pos(ix, iy)] = glm::uvec4{0, 0, 0, 0};
				t_Data[t_pos(ix, iy)+1] = glm::uvec4{t_clr.r, t_clr.g, t_clr.b, 0};
				t_Data[t_pos(ix, iy)+1].a |= (LIGHT_FULL << LIGHT_SHIFT);
			}
		}
		
		for(::std::size_t iy = 6; iy < 15; ++iy)
		{
			if(iy == 9) continue;
		
			t_Data[t_pos(9, iy)] = glm::uvec4{187, 187, 187, 61};
			t_Data[t_pos(9, iy)+1] = glm::uvec4{187, 187, 187, 0};
			t_Data[t_pos(9, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
			t_Data[t_pos(10, iy)] = glm::uvec4{187, 187, 187, 61};
			t_Data[t_pos(10, iy)+1] = glm::uvec4{187, 187, 187, 0};
			t_Data[t_pos(10, iy)+1].a |= (LIGHT_DIM << LIGHT_SHIFT);
		}
		
		
		
		
		//t_Data[t_pos(10, 7)] = glm::uvec4{255, 255, 0, 15};
		
		/*for(::std::size_t ix = 0; ix < t_Data.size(); ix+=2)
		{
			const auto t_clr = t_groundClr(t_gen);
		
			t_Data[ix] = glm::uvec4{t_clr.r, t_clr.g, t_clr.b, 219};
			t_Data[ix+1] = glm::uvec4{0, 0, 0, 0};
		}*/
		
		const unsigned t_levelIncrement = 13;//25;
		
		
		
			/*for(int ix = t_tl.x; ix <= t_br.x; ix++)
			{		
				t_Data[t_pos(ix, i)+1].a |= (t_levelIncrement * ((i % 2 == 0 && i != 0) ? (i-1)/2 : i/2));
				
				if(i > 1 && (i % 2 != 0))
				{
					t_Data[t_pos(ix, i)+1].a |= SHADOW_N;
				}
			}*/
		
		
		
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
		
		
		
		
		
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		
		ut::gl::program program{
			ut::gl::vertex_shader{ ut::gl::from_file, "assets/test.vs.glsl" },
			ut::gl::fragment_shader{ ut::gl::from_file, "assets/test.fs.glsl" },
		};

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		program.use();
		
		// Set uniforms	
		const auto t_cursorPosPos = glGetUniformLocation(program.handle(), "cursor_pos");
		const auto t_sheetDimPos = glGetUniformLocation(program.handle(), "sheet_dimensions");
		const auto t_glyphDimPos = glGetUniformLocation(program.handle(), "glyph_dimensions");
		const auto t_glyphCountPos = glGetUniformLocation(program.handle(), "glyph_count");
		const auto t_projMatPos = glGetUniformLocation(program.handle(), "projection_mat");
		
		const auto t_cursorColorPos = glGetUniformLocation(program.handle(), "cursor_default");
		
		glm::ivec2 t_cursorPos{ -1, -1 };
		glUniform2iv(t_cursorPosPos, 1, glm::value_ptr(t_cursorPos));
		
		
		const auto t_fogColorPos = glGetUniformLocation(program.handle(), "fog_color");
		const glm::vec4 t_fogClr{ 0.1f, 0.1f, 0.3f, 1.f };
		glUniform4fv(t_fogColorPos, 1, glm::value_ptr(t_fogClr));
		
		const glm::vec4 t_cursorClr{ 1.f, 1.f, 1.f, 1.f };
		glUniform4fv(t_cursorColorPos, 1, glm::value_ptr(t_cursorClr));
		
		const auto t_fogDensityPos = glGetUniformLocation(program.handle(), "fog_density");
		const float t_fogDensity = 5.f;//0.15f;
		glUniform1f(t_fogDensityPos, t_fogDensity);

		// We use an inverted ortho projection here (flipped y axis) to enable us to use cordinates the
		// same way they would be used on a 2d screen (with (0,0) in top left corner, and y increasing
		// downwards)
		const auto t_proj = glm::ortho(0.f, (float)t_width, (float)t_height, 0.f, -1.f, 1.f);	
		glUniformMatrix4fv(t_projMatPos, 1, GL_FALSE, glm::value_ptr(t_proj));
		
		glUniform2iv(t_glyphDimPos, 1, glm::value_ptr(t_glyphDim));
		glUniform2iv(t_glyphCountPos, 1, glm::value_ptr(t_glyphCount));
		
		const glm::ivec2 t_sheetDims{ texture::sheet_width, texture::sheet_height };
		glUniform2iv(t_sheetDimPos, 1, glm::value_ptr(t_sheetDims));
		
		const auto t_samplerPos = glGetUniformLocation(program.handle(), "sheet_texture");
		glUniform1i(t_samplerPos, 0);
		
		const auto t_samplerBufPos = glGetUniformLocation(program.handle(), "input_buffer");
		glUniform1i(t_samplerBufPos, 1);

		const auto t_samplerShadowPos = glGetUniformLocation(program.handle(), "shadow_texture");
		glUniform1i(t_samplerShadowPos, 2);
		
		const auto t_lightIntensityPos = glGetUniformLocation(program.handle(), "light_intensity");
		const float t_intensity = 0.7f;
		glUniform1f(t_lightIntensityPos, t_intensity);
		
		
		const auto t_useLightingPos = glGetUniformLocation(program.handle(), "use_lighting");
		glUniform1i(t_useLightingPos, useLighting);
		
		const auto t_ambientLightPos = glGetUniformLocation(program.handle(), "ambient_light");
		//const glm::vec4 t_ambient{ 0.2f, 0.2f, 0.2f, 1.f };
		const glm::vec4 t_ambient{ 0.5f, 0.5f, 0.5f, 1.f };
		glUniform4fv(t_ambientLightPos, 1, glm::value_ptr(t_ambient));
		
		
		const auto t_topleftPos = glGetUniformLocation(program.handle(), "top_left_pos");
		const glm::vec2 t_topleft{ 0.f, 0.f };
		glUniform2fv(t_topleftPos, 1, glm::value_ptr(t_topleft));
		
		const auto t_lightPos = glGetUniformLocation(program.handle(), "light_pos");
		glm::vec2 t_light{ 10.f, 7.f };
		glUniform2fv(t_lightPos, 1, glm::value_ptr(t_light));
		
		
		const auto t_lightColorPos = glGetUniformLocation(program.handle(), "light_clr");
		//const glm::vec4 t_lightClr{ 0.92f, 0.5216f, 0.1255f, 1.f };
		//const glm::vec4 t_lightClr{ 0.96f, 0.81f, 0.4549f, 1.f }; 
		
		const glm::vec4 t_lightClr{1.000f, 0.647f, 0.000f, 1.f };
		
		//const glm::vec4 t_lightClr{1.000f, 0.1f, 0.9f, 1.f };
		glUniform4fv(t_lightColorPos, 1, glm::value_ptr(t_lightClr));
		
		const auto t_lightModeDebugPos = glGetUniformLocation(program.handle(), "debug_mode");
		glUniform1i(t_lightModeDebugPos, lightModeDebug);
		
		const auto t_useDynamicPos = glGetUniformLocation(program.handle(), "use_dynamic_lighting");
		glUniform1i(t_useDynamicPos, useDynamicLighting);
		

		::std::size_t inputCounter = 0;
		const ::std::size_t inputPeriod = 4;

		::std::size_t animCounter = 0;
		const ::std::size_t animPeriod = 6;
		//const ::std::size_t animPeriod = 7;

		/*::std::size_t clrCounter = 0;
		::std::size_t numClr = 3;
		bool latch = false;
		
		glm::vec4 t_clrs[3] =
		{
			{1.000f, 0.647f, 0.000f, 1.f },
			{0.000f, 1.0f, 0.000f, 1.f },
			{1.000f, 0.0f, 0.000f, 1.f }
		};*/

		while (!glfwWindowShouldClose(window))
		{
			if(animCounter >= animPeriod)
			{
				animCounter = 0;
			
				const float t_intensityMod = t_intensityDistrib(t_gen);
				glUniform1f(t_lightIntensityPos, t_intensity * t_intensityMod);
				
				/*const double t_mod = glm::abs(glm::cos(glfwGetTime()*2.));
				
				if(t_mod <= 0.1f && !latch)
				{
					latch = true;
					clrCounter = (clrCounter + 1) % numClr;
					
					glUniform4fv(t_lightColorPos, 1, glm::value_ptr(t_clrs[clrCounter]));
				}
				
				if(t_mod >= 0.95f && latch)
					latch = false;		
				
				glUniform1f(t_lightIntensityPos, t_intensity * t_mod);*/
				
			}
			else ++animCounter;
		
			if(inputCounter >= inputPeriod)
			{			
				inputCounter = 0;
				
				if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				{
					useLighting = !useLighting;
					glUniform1i(t_useLightingPos, useLighting);
				}
			
				if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
				{
					lightModeDebug = !lightModeDebug;
					glUniform1i(t_lightModeDebugPos, lightModeDebug);
				}			
				
				if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					useDynamicLighting = !useDynamicLighting;
					glUniform1i(t_useDynamicPos, useDynamicLighting);
				}
				
				
				if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				{
					t_light.y = glm::max(t_light.y - 1.f, 6.f); 
					glUniform2fv(t_lightPos, 1, glm::value_ptr(t_light));
				}
				
				if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				{
					t_light.y = glm::min(t_light.y + 1.f, 20.f-7.f); 
					glUniform2fv(t_lightPos, 1, glm::value_ptr(t_light));
				}
				
				if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				{
					t_light.x = glm::max(t_light.x - 1.f, 6.f); 
					glUniform2fv(t_lightPos, 1, glm::value_ptr(t_light));
				}
				
				if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				{
					t_light.x = glm::min(t_light.x + 1.f, 20.f-7.f); 
					glUniform2fv(t_lightPos, 1, glm::value_ptr(t_light));
				}
				
			}
			else ++inputCounter;
			
			glUniform2iv(t_cursorPosPos, 1, glm::value_ptr(t_cursorPos));
		
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
	catch(const ut::gl::shader_exception& p_ex)
	{
		std::cout << "An exception was thrown: " << p_ex.what() << '\n' << p_ex.log() << std::endl;
	}
}
