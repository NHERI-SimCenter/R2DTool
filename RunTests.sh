#!/bin/bash 

# Script to run R2D Unit Tests
# Created by Stevan Gavrilovic

BASEDIR=$(dirname "$0")

cd $BASEDIR

echo "Script file is in directory " $PWD

cd build

# Run qmake for Tests
qmake ../Tests/R2DTests.pri
status=$?
if [[ $status != 0 ]]
then
    echo "R2D Tests: qmake failed";
    exit $status;
fi

# make
make -j8
status=$?;
if [[ $status != 0 ]]
then
    echo "R2D Tests: make failed";
    exit $status;
fi

cd ..

# Copy over the dependencies for the test app
mkdir Frameworks

cp -Rf ../QGISPlugin/mac/qgis-deps-0.9/stage/lib/* Frameworks/

cp -Rf ../QGISPlugin/mac/Install/lib/* Frameworks/


# Copy over the examples folder
mkdir ./build/Examples

cp -Rf ../R2DExamples/* ./build/Examples/

# Copy over the applications dir

cp -Rf ../SimCenterBackendApplications/applications build/

status=$?;
if [[ $status != 0 ]]
then
    echo "Error copying the applications";
    exit $status;
fi

# Run the test app
./build/R2DTest

status=$?
if [[ $status != 0 ]]
then
    echo "R2D: unit tests failed";
    exit $status;
fi

echo "All R2D Unit Tests Passed!"