#include <iostream>
#include <cstdlib>
#include <log.hxx>
#include <ut/cast.hxx>
#include <ut/format.hxx>
#include <ut/throwf.hxx>
#include <render_context.hxx>
#include <engine.hxx>


auto glfw_error_callback(int p_error, const char* p_description)
	-> void
{
	LOG_F_TAG("render_context") << "GLFW error occured: \"" << p_description << "\"";
	
	// Throw exception to be handled in the main function
	ut::throwf<::std::runtime_error>("fatal GLFW error: %s", p_description);
}

auto opengl_message_callback(	GLenum p_source, GLenum p_type, GLuint p_id,
   								GLenum p_severity, GLsizei p_length,
   								const GLchar* p_message, const void* p_userParam )
{
	// Determine message severity level
	lg::severity_level t_msgType{ };
	
	if(p_severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		t_msgType = lg::severity_level::info;
	else if(p_type == GL_DEBUG_TYPE_ERROR)
		t_msgType = lg::severity_level::error;
	else
		t_msgType = lg::severity_level::warning;
	
	// User parameter is a threshold.
	if(ut::enum_cast(t_msgType) <= ut::enum_cast(*static_cast<const lg::severity_level*>(p_userParam)))
	{
		switch(t_msgType)
		{
			case lg::severity_level::info:
				LOG_I_TAG("opengl") << p_message;
				break;
				
			case lg::severity_level::error:
				LOG_E_TAG("opengl") << p_message;
				break;
				
			case lg::severity_level::warning:
				LOG_W_TAG("opengl") << p_message;
				break;
				
			default:
				break;
		}
	}
}


render_context::render_context()
{
}

render_context::~render_context()
{
	/*if(m_Initialized)
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}*/
}

auto render_context::initialize()
	-> void
{
	LOG_D_TAG("render_context") << "initialization started";

	init_glfw();
	init_glxw();
	init_debug();
	
	m_Initialized = true;
}

auto render_context::deinitialize()
	-> void
{
	LOG_D_TAG("render_context") << "deinitialization started";
	
	// Kill the window
	glfwDestroyWindow(m_WindowHandle);
	
	// Shutdown GLFW
	glfwTerminate();
}

auto render_context::init_debug()
	-> void
{
	static lg::severity_level g_Threshold{ lg::severity_level::warning };

	glDebugMessageCallback(opengl_message_callback, &g_Threshold);
}

auto render_context::init_glfw()
	-> void
{
	if(!glfwInit())
	{
		LOG_F_TAG("render_context") << "failed to initialize GLFW";
		throw ::std::runtime_error("fatal GLFW error");
	}
	
	glfwSetErrorCallback(glfw_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	
	if(!(m_WindowHandle = glfwCreateWindow(100, 100, engine::game_information().window_title().c_str(), 0, 0)))
	{
		LOG_F_TAG("render_context") << "failed to create window";
		glfwTerminate();
		throw ::std::runtime_error("fatal GLFW error");
	}
	
	glfwMakeContextCurrent(m_WindowHandle);
}

auto render_context::init_glxw()
	-> void
{
	if(glxwInit())
	{
		LOG_F_TAG("render_context") << "failed to initialize GLXW";
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
		throw ::std::runtime_error("fatal GLXW error");
	}
	
	// Enable VSync
	glfwSwapInterval(1);
	
	report_version();
}

auto render_context::report_version()
	-> void
{
	const char* t_glStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	
	LOG_I_TAG("render_context") << "OpenGl version " << t_glStr;
}

auto render_context::handle() const
	-> handle_type
{
	return m_WindowHandle;
}

auto render_context::resize(const dimension_type& p_dim)
	-> void
{
	LOG_D_TAG("render_context") << "window size is (" << p_dim.x << ", " << p_dim.y << ")";
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
