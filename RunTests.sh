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

# Download dakota and opensees, extract them, and install them to the build/applications folder
mkdir dakota

cd dakota

curl -O  https://dakota.sandia.gov/sites/default/files/distributions/public/dakota-6.15.0-public-darwin.Darwin.x86_64-cli.tar.gz
tar -xf *.tar.gz

cd ..

mkdir  ./build/applications/dakota

status=$?;
if [[ $status != 0 ]]
then
    echo "Error making the dakota dir";
    exit $status;
fi

cp -rf ./dakota/dakota-*/* ./build/applications/dakota

status=$?;
if [[ $status != 0 ]]
then
    echo "Error copying dakota to build/applications/dakota dir";
    exit $status;
fi

mkdir opensees

cd opensees

curl -O  https://opensees.berkeley.edu/OpenSees/code/OpenSees3.3.0Mac.tar.gz

tar -xf *.tar.gz

cd ..

mkdir  ./build/applications/opensees

status=$?;
if [[ $status != 0 ]]
then
    echo "Error making the build/applications/opensees dir";
    exit $status;
fi

cp -rf ./opensees/OpenSees*/* ./build/applications/opensees

status=$?;
if [[ $status != 0 ]]
then
    echo "Error copying opensees to applications dir";
    exit $status;
fi

# Install nheri-sincenter python repositories
python3 -m pip install nheri-simcenter

# Disable gatekeeper because dakota is unsigned
sudo spctl --master-disable

# Run the test app
./build/R2DTest

status=$?
if [[ $status != 0 ]]
then
    echo "R2D: unit tests failed";
    exit $status;
fi

echo "All R2D Unit Tests Passed!"