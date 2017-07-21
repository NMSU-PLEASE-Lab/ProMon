
<<<<<<< HEAD
## Installation of Dyninst 9.3.2
=======
Installation of Dyninst 9.3.2
>>>>>>> d00d20997e4b9177c0d57a7a86ced44e33fef76e
============================
1. cmake 3.7 or newer required for boost_1_61_0:
sudo apt-get remove cmake
sudo apt-get remove cmake-data
wget https://cmake.org/files/v3.8/cmake-3.8.1-Linux-x86_64.tar.gz
tar -xvzf cmake-3.8.1-Linux-x86_64.tar.gz
cd cmake-3.8.1-Linux-x86_64
sudo cp -R * /usr/

2. Boost Installation (boost_1_61_0):
wget https://gigenet.dl.sourceforge.net/project/boost/boost/1.61.0/boost_1_61_0.tar.gz
	tar -xvzf boost_1_61_0.tar.gz
	cd boost_1_61_0
	./bootstrap.sh --prefix=/home/ujjwal/Build/boost_1_61_0
	./b2 install --with=all

3. Dyninst installation (Libelf, libdwarf, and libiberty will be automatically installed by dyninst)
wget https://github.com/dyninst/dyninst/archive/v9.3.2.tar.gz  /home/ujjwal/Build/dyninst-v9.3.2.tar.gz
cd /home/ujjwal/Build/
tar xf dyninst-v9.3.2.tar.gz
cd dyninst-v9.3.2
cmake . -DCMAKE_INSTALL_PREFIX=/home/ujjwal/Build/DyninstAPI-9.3.2 -DBoost_INCLUDE_DIR=/home/ujjwal/Build/boost_1_61_0/include -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
make
make install

## msgpack-c Installation
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.3/msgpack-2.1.3.tar.gz
tar xf msgpack-2.1.3.tar.gz
cd msgpack-2.1.3/
cmake . -DCMAKE_INSTALL_PREFIX=/home/ujjwal/Build/msgpack-2.1.3 -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
make
make install

##Promon Installation
1. Change the necessary configuration parameters in Makefile
2. make

