#!/bin/bash 

#
# create build dir if does not exist, cd to build, conan install and then qmake
# 

mkdir -p build
cd build

# conan install
conan install .. --build missing
status=$?
if [[ $status != 0 ]]
then
    echo "R2D: conan install failed";
    exit $status;
fi

# qmake
qmake ../R2D.pro
status=$?
if [[ $status != 0 ]]
then
    echo "R2D: qmake failed";
    exit $status;
fi

# make
make -j8
status=$?;
if [[ $status != 0 ]]
then
    echo "R2D: make failed";
    exit $status;
fi
