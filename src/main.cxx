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
		texture t_tex{ "assets/tex.png" };
		
		// Resize window
		const glm::ivec2 t_glyphDim = t_tex.glyph_size();
		const glm::ivec2 t_glyphCount = { 64, 64 };
		
		const auto t_width = t_glyphDim.x * t_glyphCount.x;
		const auto t_height = t_glyphDim.y * t_glyphCount.y;
		
		glfwSetWindowSize(window, t_width, t_height);
		
		
		// Glyph data set
		::std::vector<glm::uvec4> t_Data;
		t_Data.resize(t_glyphCount.x * t_glyphCount.y * 2);
		
		std::random_device rd;
		std::mt19937 t_gen(rd());
    	std::uniform_int_distribution<unsigned> t_distrib(0, 255);
		
		for(::std::size_t ix = 0; ix < t_Data.size(); ix+=2)
		{
			unsigned r = t_distrib(rd);
			unsigned g = t_distrib(rd);
			unsigned b = t_distrib(rd);
		
			t_Data[ix] = glm::uvec4{0, 0, 0, 0};
			t_Data[ix+1] = glm::uvec4{r, g, b, 0x0};
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
		
		
		
		
		
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		
		ut::gl::program program{
			ut::gl::vertex_shader{ ut::gl::from_file, "assets/test.vs.glsl" },
			ut::gl::fragment_shader{ ut::gl::from_file, "assets/test.fs.glsl" },
		};

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		program.use();
		
		// Set uniforms
		const auto t_sheetDimPos = glGetUniformLocation(program.handle(), "sheet_dimensions");
		const auto t_glyphDimPos = glGetUniformLocation(program.handle(), "glyph_dimensions");
		const auto t_glyphCountPos = glGetUniformLocation(program.handle(), "glyph_count");
		const auto t_projMatPos = glGetUniformLocation(program.handle(), "projection_mat");

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

		while (!glfwWindowShouldClose(window))
		{
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
