#!/bin/bash 

release=${1:-"NO_RELEASE"}

#
# create build dir if does not exist, cd to build, conan install and then qmake
# 

mkdir -p build
cd build

# conan install
conan install .. --build missing
cmd_status=$?
if [[ $cmd_status != 0 ]]
then
    echo "R2D: conan install failed";
    exit $cmd_status;
fi

# qmake
if [ -n "$release" ] && [ "$release" = "release" ]; then
    echo "******** RELEASE BUILD *************"    
    qmake QMAKE_CXXFLAGS+=-D_SC_RELEASE ../R2D.pro
    cmd_status=$?; if [[ $cmd_status != 0 ]]; then echo "R2D: qmake failed"; exit $cmd_status; fi        
else
    echo "********* NON RELEASE BUILD ********"
    qmake ../RD2.pro
    cmd_status=$?; if [[ $cmd_status != 0 ]]; then echo "R2D: qmake failed"; exit $cmd_status; fi    
fi

cmd_status=$?
if [[ $cmd_status != 0 ]]
then
    echo "R2D: qmake failed";
    exit $cmd_status;
fi

# make
make 
cmd_status=$?;
if [[ $cmd_status != 0 ]]
then
    echo "R2D: make failed";
    exit $cmd_status;
fi
