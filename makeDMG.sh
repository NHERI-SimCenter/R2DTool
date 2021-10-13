#!/bin/bash 

appName="R2D"
appFile=$appName".app"
dmgFile=$appName"_Mac_Download.dmg"

pathToBackendApps="/Users/fmckenna/release/SimCenterBackendApplications"
pathToOpenSees="/Users/fmckenna/bin/OpenSees3.2.2"
pathToDakota="/Users/fmckenna/dakota-6.12.0"

#
# create build dir if does not exist, cd to build, conan install and then qmake
# 

mkdir -p build
cd build
conan install .. --build missing
qmake ../$appName.pro
make

#
# remove old app and dmg file & rebuild app
#

pathApp=`pwd`/$appFile
rm -fr $appFile
rm $dmgFile
make

#
# Check to see if the app built
#

if ! [ -x "$(command -v open $pathApp)" ]; then
	echo "$appFile did not build. Exiting."
	exit 
fi

#
# macdeployqt it
#

macdeployqt $appFile

#
# now ESRI stuff from Steve
#

# Define the paths to the application and to libEsriCommonQt.dylib - this should not change
pathAppBin=$pathApp/Contents/MacOS/R2D
pathAppLib=$pathApp/Contents/Frameworks/libEsriCommonQt.dylib

# Get the paths that are in the libraries - these paths will be changed to relative paths instead of the absolute paths
pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

echo $pathEsriCommonQt
echo $pathLibruntimecore

# Use install name tool to change these to relative paths
install_name_tool -change $pathEsriCommonQt @rpath/libEsriCommonQt.dylib $pathAppBin
install_name_tool -change $pathLibruntimecore @rpath/libruntimecore.dylib $pathAppLib

# Check to make sure it worked
pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

if [ "$pathEsriCommonQt" != "@rpath/libEsriCommonQt.dylib" ]; then
    echo "Failed to change the path $pathEsriCommonQt"
	exit
fi

if [ "$pathLibruntimecore" != "@rpath/libruntimecore.dylib" ]; then
    echo "Failed to change the path $pathLibruntimecore"
	exit
fi

#
# copy needed file from SimCenterBackendApplications
#


mkdir  $pathApp/Contents/MacOS/Examples
mkdir  $pathApp/Contents/MacOS/Databases
cp     ../../R2DExamples/Examples.json $pathApp/Contents/MacOS/Examples
cp -fR $pathToBackendApps/applications $pathApp/Contents/MacOS
cp -fR $pathToBackendApps/applications/performRegionalEventSimulation/regionalWindField/database/historical_storm/* $pathApp/Contents/MacOS/Databases
mkdir  $pathApp/Contents/MacOS/applications/opensees
mkdir  $pathApp/Contents/MacOS/applications/dakota
cp -fr $pathToOpenSees/* $pathApp/Contents/MacOS/applications/opensees
cp -fr $pathToDakota/* $pathApp/Contents/MacOS/applications/dakota

# remove unwanted to make this thing smaller

declare -a notWantedApp=("createSAM/mdofBuildingModel/" 
			 "createSAM/openSeesInput"
			 "createSAM/AutoSDA"			 
			 "createEVENT/ASCE7_WindSpeed"
			 "createEVENT/CFDEvent"
			 "createEVENT/HighRiseTPU"
			 "createEVENT/LowRiseTPU"
			 "createEVENT/NonisolatedLowRiseTPU"
			 "createEVENT/stochasticWind"
			 "createEVENT/stochasticGroundMotion"
			 "createEVENT/windTunnelExperiment"
			 "createEDP/standardWindEDP"
			 "createEDP/standardEarthquakeEDP"
			 "createEDP/userEDP"
			)

for app in "${notWantedApp[@]}"
do
   echo "removing $app"
   rm -fr $pathApp/Contents/MacOS/applications/$app
done


#
# now before we codesign and verify, check userID file exists
#

userID="../userID.sh"

if [ ! -f "$userID" ]; then
    
    echo "creating dmg $dmgFile"
    hdiutil create $dmgFile -fs HFS+ -srcfolder ./$appFile -format UDZO -volname $appName

    echo "No password & credential file to continue with codesig and App store verification"
    exit
fi

source $userID
echo $appleID

#
# codesign all files, create dmg and then code sign that too
#

# to codesign need a certificate from the apple developer program (one per user)
# create one
# then download certificate & get public key

echo "codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $appFile"
codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $appFile

# create dmg

echo "hdiutil create $dmgFile -fs HFS+ -srcfolder ./$appFile -format UDZO -volname $appName"
hdiutil create $dmgFile -fs HFS+ -srcfolder ./$appFile -format UDZO -volname $appName

#codesign dmg
echo "codesign --force --sign "$appleCredential" $dmgFile"
codesign --force --sign "$appleCredential" $dmgFile

echo "Issue the following: " 
echo "xcrun altool --notarize-app -u $appleID -p $appleAppPassword -f ./$dmgFile --primary-bundle-id altool --verbose"
echo ""
echo "returns id: ID .. wait for email indicating success"
echo "To check status"
echo "xcrun altool --notarization-info ID  -u $appleID  -p $appleAppPassword"
echo ""
echo "Finally staple the dmg"
echo "xcrun stapler staple \"$appName\" $dmgFile"
