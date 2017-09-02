#include <iostream>
#include <cstdlib>
#include <ut/cast.hxx>
#include <ut/format.hxx>
#include <render_context.hxx>
#include <diagnostics.hxx>


auto glfw_error_callback(int p_error, const char* p_description)
	-> void
{
	::std::cerr << "glfw: error: " << p_description << ::std::endl;
	::std::exit(EXIT_FAILURE);
}

auto opengl_message_callback(	GLenum p_source, GLenum p_type, GLuint p_id,
   								GLenum p_severity, GLsizei p_length,
   								const GLchar* p_message, const void* p_userParam )
{
	// Determine message severity level
	message_type t_msgType{ };
	
	if(p_severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		t_msgType = message_type::info;
	else if(p_type == GL_DEBUG_TYPE_ERROR)
		t_msgType = message_type::error;
	else
		t_msgType = message_type::warning;
	
	// User parameter is a threshold.
	if(ut::enum_cast(t_msgType) <= ut::enum_cast(*static_cast<const message_type*>(p_userParam)))
	{
		post_diagnostic(t_msgType, "opengl", p_message);
	}
}


render_context::render_context()
{
}

render_context::~render_context()
{
	if(m_Initialized)
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}
}

auto render_context::initialize()
	-> void
{
	init_glfw();
	init_glxw();
	init_debug();
	
	m_Initialized = true;
}

auto render_context::init_debug()
	-> void
{
	static message_type g_Threshold{ message_type::warning };

	glDebugMessageCallback(opengl_message_callback, &g_Threshold);
}

auto render_context::init_glfw()
	-> void
{
	if(!glfwInit())
	{
		post_diagnostic(message_type::error, "glfw", "failed to initialize");
		::std::exit(EXIT_FAILURE);
	}
	
	glfwSetErrorCallback(glfw_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	
	if(!(m_WindowHandle = glfwCreateWindow(100, 100, "gl_app", 0, 0)))
	{
		post_diagnostic(message_type::error, "glfw", "failed to create window");
		glfwTerminate();
		::std::exit(EXIT_FAILURE);
	}
	
	glfwMakeContextCurrent(m_WindowHandle);
}

auto render_context::init_glxw()
	-> void
{
	if(glxwInit())
	{
		post_diagnostic(message_type::error, "glxw", "failed to initialize");
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
		::std::exit(EXIT_FAILURE);
	}
	
	// Enable VSync
	glfwSwapInterval(1);
	
	report_version();
}

auto render_context::report_version()
	-> void
{
	const char* t_glStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	post_diagnostic(message_type::info, "glxw", ut::sprintf("OpenGL version %s", t_glStr));
}

auto render_context::handle() const
	-> handle_type
{
	return m_WindowHandle;
}

auto render_context::resize(const dimension_type& p_dim)
	-> void
{
	glfwSetWindowSize(m_WindowHandle, p_dim.x, p_dim.y);
	
	// Reset viewport
	// const auto t_ratio = p_dim.x / static_cast<float>(p_dim.y);
	glViewport(0, 0, p_dim.x, p_dim.y);
	
	m_WindowSize = p_dim;
}

auto render_context::dimensions() const
	-> const dimension_type&
{
	return m_WindowSize;
}

auto render_context::end_frame()
	-> void
{
	// TODO maybe this should be done in context.
	glfwSwapBuffers(handle());
}

auto render_context::begin_frame()
	-> void
{
	glClear(GL_COLOR_BUFFER_BIT);
}

auto render_context::should_close() const
	-> bool
{
	return glfwWindowShouldClose(m_WindowHandle);
}

auto render_context::pump_events()
	-> void
{
	glfwPollEvents();
}
