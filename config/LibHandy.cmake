
list(APPEND CGTK_COMPILE_DEFINITIONS HANDY_USE_UNSTABLE_API)

find_library(LIBHANDY_LIB_ZERO handy-0.0)
find_library(LIBHANDY_LIB_ONE handy-1)

if ((NOT LIBHANDY_LIB_ZERO) AND (NOT LIBHANDY_LIB_ONE))
	set(libhandy_submodule_path "${CMAKE_SOURCE_DIR}/lib/libhandy")
	
	if (NOT EXISTS ${libhandy_submodule_path})
		execute_process(
				COMMAND git submodule init
				COMMAND git submodule update lib/libhandy
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)
	endif()
	
	if (NOT EXISTS ${libhandy_submodule_path}/build)
		execute_process(
				COMMAND mkdir build
				COMMAND meson setup build/
				WORKING_DIRECTORY ${libhandy_submodule_path}
		)
	endif()
	
	execute_process(
			COMMAND ninja
			WORKING_DIRECTORY ${libhandy_submodule_path}/build
	)
	
	find_library(LIBHANDY_LIB_ZERO handy-0.0 PATHS ${libhandy_submodule_path}/build/src)
	find_library(LIBHANDY_LIB_ONE handy-1 PATHS ${libhandy_submodule_path}/build/src)
	
	unset(libhandy_submodule_path)
endif()

if (NOT LIBHANDY_LIB_ONE)
	if(NOT LIBHANDY_LIB_ZERO)
		message(FATAL_ERROR "Important library was not found: handy")
	else()
		set(LIBHANDY_LIB ${LIBHANDY_LIB_ZERO})
		
		list(APPEND CGTK_COMPILE_DEFINITIONS HANDY_USE_ZERO_API)
	endif()
else()
	set(LIBHANDY_LIB ${LIBHANDY_LIB_ONE})
endif()

list(APPEND CGTK_LIBRARIES ${LIBHANDY_LIB})
