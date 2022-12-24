#!/bin/bash

RES_DIR=res

# install dependencies
sudo apt-get update

# Ubuntu 14
# sudo apt-get install -y g++ make libssl-dev autoconf libtool yasm libz-dev gcc-multilib g++-multilib libxrender1 libxtst6 libXi6 build-essential libboost-system-dev libboost-thread-dev libboost-program-options-dev libboost-test-dev libeigen3-dev zlib1g-dev libbz2-dev liblzma-dev

# Ubuntu 18
sudo apt-get install -y g++ make libssl-dev autoconf libtool yasm libz-dev gcc-multilib g++-multilib libxrender1 libxtst6 build-essential libboost-system-dev libboost-thread-dev libboost-program-options-dev libboost-test-dev libeigen3-dev zlib1g-dev libbz2-dev liblzma-dev


# edit ~/.bashrc in advance
sudo echo "export LD_LIBRARY_PATH=/usr/local/ssl/include/openssl:/usr/lib:/usr/local/lib:/usr/lib/pkgconfig:/usr/local/include/wx-2.8/wx:\$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=/usr/lib/pkgconfig
export OPENSSL_ROOT_DIR=/usr/local/ssl
export OPENSSL_LIBRARIES=/usr/local/ssl/lib/

PATH=/usr/local/ssl/bin:\$PATH

export PATH=\$PATH:/usr/local/cmake/bin" >> ~/.bashrc

source ~/.bashrc

cd $RES_DIR

# install cmake 3.22.0
tar -zxvf cmake-3.22.4.tar.gz
cd cmake-3.22.4/
./bootstrap
make
sudo make install

sudo ln -s /usr/local/bin/cmake /usr/bin/

cd ..

# install hiredis
tar -zxvf hiredis-1.0.2.tar.gz
cd hiredis-1.0.2/
make 
sudo make install

cd ..

# install gf-complete
tar -zxvf gf-complete.tar.gz
cd gf-complete/
chmod +x ./autogen.sh
./autogen.sh
./configure
make
sudo make install

cd ..

# install ISA-L
tar -zxvf isa-l-2.30.0.tar.gz
cd isa-l-2.30.0/
./autogen.sh
./configure
make
sudo make install

cd ..

source ~/.bashrc

# install protobuf 2.5.0
tar -zxvf protobuf-2.5.0.tar.gz
cd protobuf-2.5.0/
./autogen.sh
./configure
make
sudo make install

cd ..

# sudo apt-get install -y zip
# unzip openec.zip
# cd openec/
# cd hdfs3-integration/
# source ~/.bashrc
# ./install.sh

# cd ..
