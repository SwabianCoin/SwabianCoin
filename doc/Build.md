
# SwabianCoin Build instructions

![Logo](swabiancoin_logo.png "SwabianCoin")

## Table of Contents

 - [Ubuntu](#ubuntu)
 - [Windows](#windows-experimental)

## Ubuntu
### Dependencies

 - **\[Ubuntu 18.04\]** Install apt packages
```
sudo apt-get update
sudo apt-get install libboost-all-dev libssl-dev libgflags2.2
```

 - **\[Ubuntu 18.04\]** Install up-to-date cmake 
```
wget https://github.com/Kitware/CMake/releases/download/v3.15.5/cmake-3.15.5-Linux-x86_64.sh
bash ./cmake-3.15.5-Linux-x86_64.sh
sudo ln -s path_to_cmake/bin/* /usr/local/bin/
```

Logout/Login after that.

### Build

Starting from the SwabianCoin repository root directory:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

That's it for the executable. If you need unit tests, go on with the next step

### Unit Tests

#### **\[Ubuntu 18.04\]** Install GTest

```
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo make install
```

#### Build and Run Tests

Starting from the build directory of chapter [Build](#build):
```
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_UNIT_TESTS=ON ..
make
./runTests
```
Watch the output for any failing tests.

## Windows \[Experimental\]

Note: Building in Windows has been done successfully (with a lot of compiler warnings) using the following guide but is not officially supported. Compatibility may change in the future.

### Dependencies

 - Download and install Visual Studio 2017 (the Community Edition is fine).
 
 - Download and install Boost 1.68.0 precompiled binaries (e.g. from <https://sourceforge.net/projects/boost/files/boost-binaries/>).
 
 - Download and install OpenSSL v1.1.1d (e.g. from <https://slproweb.com/products/Win32OpenSSL.html>).
 
 - Download and install CMake (<https://cmake.org/download/>). Add the CMake bin directory to your PATH environment variable.
 
### Build

Starting from the SwabianCoin repository root directory in windows command line (or e.g. git bash):

```
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```
