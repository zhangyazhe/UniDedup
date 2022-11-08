#!/bin/bash

wget http://www.openssl.org/source/openssl-1.0.2d.tar.gz
tar -zxvf openssl-1.0.2d.tar.gz
cd openssl-1.0.2d
./config --prefix=/usr/local --openssldir=/usr/local/openssl
make && sudo make install

sudo apt-get install -y libssl-dev libcurl4-openssl-dev libgoogle-perftools-dev zlib1g-dev libffi-dev libglib2.0-dev autoconf

sudo cp -r /usr/include/glib-2.0/* /usr/local/include
sudo cp /usr/lib/x86_64-linux-gnu/glib-2.0/include/glibconfig.h /usr/local/include

sudo ln -s /usr/lib/x86_64-linux-gnu/libglib-2.0.so /usr/local/lib/libglib.so

sudo mv /usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libcrypto.a.old
sudo mv /usr/lib/x86_64-linux-gnu/libcrypto.so /usr/lib/x86_64-linux-gnu/libcrypto.so.old