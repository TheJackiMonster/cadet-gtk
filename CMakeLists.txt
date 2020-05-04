cmake_minimum_required(VERSION 3.17)
project(cadet-gtk C)

set(CMAKE_C_STANDARD 11)

include(config/GTK.cmake)
include(config/LibHandy.cmake)
include(config/GNUnet.cmake)
include(config/Jansson.cmake)

include_directories(${CGTK_INCLUDE_DIRECTORIES})

link_libraries(${CGTK_LINK_LIBRARIES})

add_definitions(${CGTK_DEFINITIONS})
add_compile_definitions(${CGTK_COMPILE_DEFINITIONS})

add_executable(cadet-gtk
		
		src/config.h
		src/main.c
		src/gui/chat.h
		src/gui/chat.c
		src/gui/contacts.h
		src/gui/contacts.c
		src/gui/util.h
		src/gui/util.c
		src/gui.h
		src/gui.c
		src/messaging.h
		src/messaging.c
		src/gnunet.h
		src/gnunet.c
		src/gtk.h
		src/gtk.c
		src/json.h
		src/json.c
)

target_link_libraries(cadet-gtk ${CGTK_LIBRARIES})

install(TARGETS cadet-gtk RUNTIME DESTINATION bin)
