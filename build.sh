#!/bin/sh

found=false

# Iterate through PKG_CONFIG_PATH to determine if gnunetarm.pc is in path
# The reason we check PKG_CONFIG_PATH first is that find is pretty slow
# No reason to add the overhead unless gnunetarm.pc is not in path
for p in ${PKG_CONFIG_PATH//:/ }; do
	# if it is, then set found to true and break
	if [[ -e "${p}/gnunetarm.pc" ]]; then
		found=true
		break;
	fi
done

# If we didn't find gnunetarm.pc in path, then add to path
if [[ "${found}" = false ]]; then
	# Check and make sure gnunetarm.pc exists
	# Redirect stderr to null, we don't actually care about the output
	path=$(find / -name "gnunetarm.pc" 2>/dev/null | head -n 1)

	# If not, exit with error
	if [[ -z "${path}" ]]; then
		echo "Unable to find the path to gnunetarm.pc!"
		exit 1
	fi

	# Get directory in which gnunetarm.pc resides
	path=$(echo "${path}" | sed s,"/gnunetarm.pc",,)
	# If empty, then don't add : to beginning
	if [[ -z "${PKG_CONFIG_PATH}" ]]; then
		PKG_CONFIG_PATH=${path}
	else
		PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:${path}
	fi
	export PKG_CONFIG_PATH
fi

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
