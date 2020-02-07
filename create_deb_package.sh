#!/bin/bash

sudo rm -rf build
mkdir build
cd build

cp ../description-pak .

cmake -DCMAKE_BUILD_TYPE=Release ..
make

echo "Don't forget to specify the correct version in checkinstall!"
sleep 3
sudo checkinstall -D --pkgname=swabiancoin --pkgsource=swabiancoin --maintainer=swabiancoin@gmail.com --requires="libboost-all-dev \(\>= 1.65\),libssl1.1 \(\>= 1.1\),libgflags2.2 \(\>= 2.2\)"

cd ..
