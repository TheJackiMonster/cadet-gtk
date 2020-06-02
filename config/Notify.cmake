
find_library(NOTIFY_LIB notify)

if (NOT NOTIFY_LIB)
	message(FATAL_ERROR "Important library was not found: notify")
endif()

list(APPEND CGTK_LIBRARIES ${NOTIFY_LIB})
