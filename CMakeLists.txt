cmake_minimum_required(VERSION 3.17)
project(cadet-gtk C)

find_package(PkgConfig REQUIRED)

if (PKG_CONFIG_FOUND)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	
	set(GTK_INCLUDE_DIR ${GTK3_INCLUDE_DIRS})
	set(GTK_LIBRARIES ${GTK3_LIBRARIES})
else()
	find_package(GTK REQUIRED)
endif()

add_compile_definitions(HANDY_USE_UNSTABLE_API)

if (EXISTS "/usr/lib/libhandy-1.so")
	set(LIBHANDY_LIB "libhandy-1.so")
else()
	add_compile_definitions(HANDY_USE_ZERO_API)
	set(LIBHANDY_LIB "libhandy-0.0.so")
endif()

list(APPEND GNUNET_LIBRARIES "libgnunetcadet.so")
list(APPEND GNUNET_LIBRARIES "libgnunetutil.so")

set(CMAKE_C_STANDARD 11)

include_directories(${GTK_INCLUDE_DIR})

if (PKG_CONFIG_FOUND)
	link_libraries(${GTK3_LIBRARY_DIRS})
	
	add_definitions(${GTK3_CFLAGS_OTHER})
endif()

add_executable(cadet-gtk src/main.c src/chat.h src/chat.c src/contacts.h src/contacts.c src/handy_ui.h src/handy_ui.c src/messaging.h src/messaging.c src/gnunet.h src/gnunet.c src/gtk.h src/gtk.c src/config.h)

target_link_libraries(cadet-gtk ${GTK_LIBRARIES} ${LIBHANDY_LIB} ${GNUNET_LIBRARIES})

install(TARGETS cadet-gtk RUNTIME DESTINATION bin)
