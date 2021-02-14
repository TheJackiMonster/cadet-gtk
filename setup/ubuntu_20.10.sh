#!/bin/bash
echo "WARNING: Please notice this script is only for testing the current state from source and not a proper release! So you should expect instability!"

echo "Do you still want to continue installing? (y/n)"
read -n1 ans
if [ $ans != 'y' ]; then exit; fi

sudo apt-get update # make sure to get newest version of packages

# These packages are necessary for the application itself
sudo apt-get install gnunet libgtk-3-0 libhandy-1-0 libjansson4 libnotify4

# *-dev packages are only necessary for compilation from sources
sudo apt-get install pkg-config gnunet-dev libgtk-3-dev libhandy-1-dev libjansson-dev libnotify-dev

# Building and installing Cadet-GTK
cd ..

sh build.sh release
sudo sh install.sh /usr
