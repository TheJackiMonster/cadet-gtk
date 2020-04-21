
find_library(GNUNET_LIB_CADET gnunetcadet)
find_library(GNUNET_LIB_UTIL gnunetutil)

if (NOT GNUNET_LIB_CADET)
	message(FATAL_ERROR "Important library was not found: gnunetcadet")
elseif(NOT GNUNET_LIB_UTIL)
	message(FATAL_ERROR "Important library was not found: gnunetutil")
else()
	list(APPEND GNUNET_LIBRARIES ${GNUNET_LIB_CADET})
	list(APPEND GNUNET_LIBRARIES ${GNUNET_LIB_UTIL})
endif()

list(APPEND CGTK_LIBRARIES ${GNUNET_LIBRARIES})
