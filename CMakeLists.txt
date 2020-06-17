cmake_minimum_required(VERSION 3.13)
project(cadet-gtk C)

set(CMAKE_C_STANDARD 11)

include(config/GTK.cmake)
include(config/LibHandy.cmake)
include(config/GNUnet.cmake)
include(config/Jansson.cmake)
include(config/Notify.cmake)

include_directories(${CGTK_INCLUDE_DIRECTORIES})

link_libraries(${CGTK_LINK_LIBRARIES})

add_definitions(${CGTK_DEFINITIONS})
add_compile_definitions(${CGTK_COMPILE_DEFINITIONS})

add_executable(cadet-gtk
		
		src/config/file.h
		src/config/file.c
		src/config/internal.h
		src/config.h
		
#		src/gnunet/connection.c                 #: internally-included
#		src/gnunet/group.c                      #: internally-included
#		src/gnunet/name.c                       #: internally-included
		src/gnunet.h
		src/gnunet.c
		
		src/gtk.h
		src/gtk.c
		
#		src/gui/dialog/chat_management.c        #: internally-included
#		src/gui/dialog/contacts_id_search.c     #: internally-included
#		src/gui/dialog/contacts_identity.c      #: internally-included
#		src/gui/dialog/contacts_new_contact.c   #: internally-included
		src/gui/chat.h
		src/gui/chat.c
		src/gui/contacts.h
		src/gui/contacts.c
		src/gui/files.h
		src/gui/files.c
		src/gui/keys.h
		src/gui/keys.c
		src/gui/notification.h
		src/gui/notification.c
		src/gui/util.h
		src/gui/util.c
		src/gui.h
		src/gui.c
		
		src/main.c
		
#		src/messaging/gnunet.c                  #: internally-included
#		src/messaging/gnunet.h                  #: internally-included
#		src/messaging/gui.c                     #: internally-included
#		src/messaging/gui.h                     #: internally-included
		src/messaging.h
		src/messaging.c
		
		src/msg.h
		src/msg.c
		
		src/storage/archive.h
		src/storage/archive.c
		src/storage/chat.h
		src/storage/chat.c
		src/storage/contacts.h
		src/storage/contacts.c
		src/storage/download.h
		src/storage/download.c
		src/storage/files.h
		src/storage/files.c
		src/storage/keys.h
		src/storage/keys.c
		src/storage/upload.h
		src/storage/upload.c
		src/storage.h
		src/storage.c
)

target_link_libraries(cadet-gtk ${CGTK_LIBRARIES})

install(TARGETS cadet-gtk RUNTIME DESTINATION bin)
