
list(APPEND GNUNET_LIBS gnunetcadet)
list(APPEND GNUNET_LIBS gnunetregex)
list(APPEND GNUNET_LIBS gnunetutil)

foreach(GNUNET_LIB_NAME ${GNUNET_LIBS})
	unset(GNUNET_LIB CACHE)
	find_library(GNUNET_LIB ${GNUNET_LIB_NAME})
	
	if (NOT GNUNET_LIB)
		message(FATAL_ERROR "Important library was not found: ${GNUNET_LIB_NAME}")
	else()
		list(APPEND GNUNET_LIBRARIES ${GNUNET_LIB})
	endif()
endforeach()

list(APPEND CGTK_LIBRARIES ${GNUNET_LIBRARIES})
