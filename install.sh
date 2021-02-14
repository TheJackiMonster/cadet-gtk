#!/bin/sh
binary="cadet-gtk"

if [ $# -gt 0 ]; then
	cp $binary "$1/bin/$binary"
else
	echo 'Installation path is required!'
fi
