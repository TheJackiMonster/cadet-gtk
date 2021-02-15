#!/bin/sh
gtk_libs=$(pkg-config --cflags --libs libhandy-1 gtk+-3.0 jansson libnotify)
gnunet_libs=$(pkg-config --cflags --libs gnunetarm gnunetcadet gnunetfs gnunetregex gnunetutil)

sources="
src/config/file.c
src/gnunet.c 
src/gtk.c 
src/gui/chat.c 
src/gui/contacts.c 
src/gui/files.c 
src/gui/keys.c 
src/gui/notification.c 
src/gui/util.c 
src/gui.c 
src/main.c 
src/messaging.c 
src/msg.c 
src/storage/archive.c 
src/storage/chat.c 
src/storage/contacts.c 
src/storage/download.c 
src/storage/files.c 
src/storage/keys.c 
src/storage/upload.c 
src/storage.c 
"

binary="cadet-gtk"
optimization=$(if [ $# -gt 0 ] && [ "$1" = "release" ]; then echo '-O2 '; elif [ $# -gt 0 ] && [ "$1" = "debug" ]; then echo '-Og -Wall '; else echo '-O0 '; fi)
ubuntu=$(if [ -z "$(gcc --version | grep Ubuntu)" ]; then echo " "; else echo "-Wl,--no-as-needed "; fi)

gcc $ubuntu $optimization $gtk_libs $gnunet_libs $sources -o $binary
