#!/bin/bash
echo "WARNING: Please notice this script is only for testing the current state from source and not a proper release! So you should expect instability!"

echo "Do you still want to continue installing? (y/n)"
read -n1 ans
if [ $ans != 'y' ]; then exit; fi

sudo apt-get update # make sure to get newest version of packages

# These packages are necessary to install current versions of GNUnet
# Original reference: https://gnunet.org/de/install-on-ubuntu1804.html
sudo apt install git libtool autoconf autopoint libargon2-dev \
	build-essential libgcrypt20-dev libidn2-dev zlib1g-dev \
	libunistring-dev libglpk-dev miniupnpc libextractor-dev \
	libjansson-dev libcurl4-gnutls-dev gnutls-bin libsqlite3-dev \
	openssl libnss3-tools libmicrohttpd-dev libopus-dev libpulse-dev \
	libogg-dev libsodium-dev 

# Building and installing GNUnet
git clone --depth 1 https://gnunet.org/git/gnunet.git
cd gnunet

export GNUNET_PREFIX=/usr/local # for example, other locations possible
./bootstrap
./configure --prefix=$GNUNET_PREFIX --disable-documentation --with-microhttpd=/opt/libmicrohttpd

sudo addgroup gnunetdns
sudo adduser --system --group --disabled-login --home /var/lib/gnunet gnunet
sudo usermod -aG gnunet $USER

make -j$(nproc || echo -n 1)
sudo make install
cd ..

# These packages are necessary for the application itself
sudo apt-get install libgtk-3-0 libhandy-0.0-0 libjansson4 libnotify4

# *-dev packages are only necessary for compilation from sources
sudo apt-get install libgtk-3-dev libhandy-0.0-dev libjansson-dev libnotify-dev

# Building and installing Cadet-GTK
mkdir ../build
cd ../build

cmake ..
make -j$(nproc || echo -n 1)
sudo make install
