
# SwabianCoin Build instructions

![Logo](swabiancoin_logo.png "SwabianCoin")

## Table of Contents

 - [Dependencies](#dependencies)
 - [Build](#build)
 - [Unit Tests](#unit-tests)

## Dependencies

 - **\[Ubuntu 18.04\]** Install apt packages
```
sudo apt-get update
sudo apt-get install libboost-all-dev libssl-dev libcurl4-openssl-dev
```

 - **\[Ubuntu 18.04\]** Install up-to-date cmake 
```
wget https://github.com/Kitware/CMake/releases/download/v3.15.5/cmake-3.15.5-Linux-x86_64.sh
bash ./cmake-3.15.5-Linux-x86_64.sh
sudo ln -s path_to_cmake/bin/* /usr/local/bin/
```

Logout/Login after that.

## Build

Starting from the root directory:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

That's it for the executable. If you need unit tests, go on with the next step

## Unit Tests

### **\[Ubuntu 18.04\]** Install GTest

```
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo make install
```

### Build and Run Tests

Starting from the build directory of chapter [Build](#build):
```
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_UNIT_TESTS=ON ..
make
./runTests
```
Watch the output for any failing tests.