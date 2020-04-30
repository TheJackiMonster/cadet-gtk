
find_library(JANSSON_LIB jansson)

if (NOT JANSSON_LIB)
	message(FATAL_ERROR "Important library was not found: jansson")
endif()

list(APPEND CGTK_LIBRARIES ${JANSSON_LIB})
