
find_library(GNUNET_LIB_CADET gnunetcadet)
find_library(GNUNET_LIB_UTIL gnunetutil)

if ((NOT GNUNET_LIB_CADET) AND (NOT GNUNET_LIB_UTIL))
	set(gnunet_submodule_path "${CMAKE_SOURCE_DIR}/lib/gnunet")
	
	if (NOT EXISTS ${gnunet_submodule_path})
		execute_process(
				COMMAND git submodule init
				COMMAND git submodule update lib/gnunet
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)
	endif()

	if (NOT EXISTS ${gnunet_submodule_path}/configure)
		execute_process(
				COMMAND ./bootstrap
				WORKING_DIRECTORY ${gnunet_submodule_path}
		)
	endif()

	if (NOT EXISTS ${gnunet_submodule_path}/Makefile)
		execute_process(
				COMMAND ./configure
				WORKING_DIRECTORY ${gnunet_submodule_path}
		)
	endif()

	execute_process(
			COMMAND make
			COMMAND sudo make install
			WORKING_DIRECTORY ${gnunet_submodule_path}
	)
	
	find_library(GNUNET_LIB_CADET gnunetcadet)
	find_library(GNUNET_LIB_UTIL gnunetutil)
	
	unset(gnunet_submodule_path)
endif()

if (NOT GNUNET_LIB_CADET)
	message(FATAL_ERROR "Important library was not found: gnunetcadet")
elseif(NOT GNUNET_LIB_UTIL)
	message(FATAL_ERROR "Important library was not found: gnunetutil")
else()
	list(APPEND GNUNET_LIBRARIES ${GNUNET_LIB_CADET})
	list(APPEND GNUNET_LIBRARIES ${GNUNET_LIB_UTIL})
endif()

list(APPEND CGTK_LIBRARIES ${GNUNET_LIBRARIES})
