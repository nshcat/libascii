# Version requirement and project info
cmake_minimum_required(VERSION 3.1.3)
project(minitrace)


# Avoid multiple definitions of target
if (NOT TARGET minitrace)	
	# Set user variables
	set(LIBMINITRACE_LIBRARIES minitrace CACHE INTERNAL "")
	
	# Options
	option(LIBMINITRACE_ENABLE "Enable minitrace" OFF)
	
	# Set source files
	file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false src/*.c)	

	# Build as library
	add_library(minitrace ${SOURCE_FILES})

	# Add top include directory as public dependency.
	# This automatically adds them to parent projects when linking to libut.
	target_include_directories(minitrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
	# Set preprocessor defines
	if(LIBMINITRACE_ENABLE)
		target_compile_definitions(minitrace PUBLIC MTR_ENABLED=1)
	endif()
	
	# Link against libpthread if on posix or msys
	if((NOT WIN32) OR MSYS)
		target_link_libraries(minitrace pthread)
	endif()
endif()
