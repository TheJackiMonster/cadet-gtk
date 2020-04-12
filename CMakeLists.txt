cmake_minimum_required(VERSION 3.17)
project(cadet_gtk C)

find_package(PkgConfig REQUIRED)

if (PKG_CONFIG_FOUND)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	
	set(GTK_INCLUDE_DIR ${GTK3_INCLUDE_DIRS})
	set(GTK_LIBRARIES ${GTK3_LIBRARIES})
else()
	find_package(GTK REQUIRED)
endif()

set(LIBHANDY_LIB "libhandy-0.0.so")

list(APPEND GNUNET_LIBRARIES "libgnunetcadet.so")
#list(APPEND GNUNET_LIBRARIES "/usr/lib/libgnunetcore.so")
list(APPEND GNUNET_LIBRARIES "/usr/lib/libgnunetutil.so")

set(CMAKE_C_STANDARD 11)

include_directories(${GTK_INCLUDE_DIR})

if (PKG_CONFIG_FOUND)
	link_libraries(${GTK3_LIBRARY_DIRS})
	
	add_definitions(${GTK3_CFLAGS_OTHER})
endif()

add_executable(cadet_gtk src/main.c src/chat.h src/chat.c src/contacts.h src/contacts.c src/handy_ui.h src/handy_ui.c)

target_link_libraries(cadet_gtk ${GTK_LIBRARIES} ${LIBHANDY_LIB} ${GNUNET_LIBRARIES} pthread)
