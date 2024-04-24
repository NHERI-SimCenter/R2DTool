#!/bin/bash 

#
# parse args
#

DMG_METHOD="NEW"

for arg in "$@"
do
    if [ "$arg" == "--old" ] || [ "$arg" == "-o" ] || [ $arg == "-OLD" ]; then
	DMG_METHOD="OLD"
    fi
done

#
# Paramaters
#

APP_NAME="R2D"
APP_FILE=$APP_NAME".app"
DMG_FILENAME=$APP_NAME"_Mac_Download.dmg"

pathToBackendApps="/Users/fmckenna/NHERI/SimCenterBackendApplications"
pathToOpenSees="/Users/fmckenna/bin/OpenSees3.6.0"
pathToDakota="/Users/fmckenna/dakota-6.12.0"
pathApp=`pwd`/build/$APP_FILE


QTDIR="/Users/fmckenna/Qt/5.15.2/clang_64/"

#
# create build dir if does not exist, remove any old app or dmg, cd to build, conan install and then qmake
# 

mkdir -p build

rm -fr ./build/$APP_FILE ./build/$DMG_FILENAME

cd build
conan install .. --build missing
qmake ../$APP_NAME.pro
make

#
# Check to see if the app built
#

if ! [ -x "$(command -v open ./$APP_FILE)" ]; then
	echo "$APP_FILE did not build. Exiting."
	exit 
fi


#
# macdeployqt it
#

macdeployqt $APP_FILE

#
# now ESRI stuff from Steve
#

# Define the paths to the application and to libEsriCommonQt.dylib - this should not change
#pathAppBin=$pathApp/Contents/MacOS/R2D
#pathAppLib=$pathApp/Contents/Frameworks/libEsriCommonQt.dylib

# Get the paths that are in the libraries - these paths will be changed to relative paths instead of the absolute paths
#pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
#pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

#echo $pathEsriCommonQt
#echo $pathLibruntimecore

# Use install name tool to change these to relative paths
#install_name_tool -change $pathEsriCommonQt @rpath/libEsriCommonQt.dylib $pathAppBin
#install_name_tool -change $pathLibruntimecore @rpath/libruntimecore.dylib $pathAppLib

# Check to make sure it worked
#pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
#pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

#if [ "$pathEsriCommonQt" != "@rpath/libEsriCommonQt.dylib" ]; then
#    echo "Failed to change the path $pathEsriCommonQt"
#	exit
#fi

#if [ "$pathLibruntimecore" != "@rpath/libruntimecore.dylib" ]; then
#    echo "Failed to change the path $pathLibruntimecore"
#	exit
#fi

#
# copy needed file from SimCenterBackendApplications
#


mkdir  $pathApp/Contents/MacOS/Examples
mkdir  $pathApp/Contents/MacOS/Databases
cp -fR $pathToBackendApps/applications $pathApp/Contents/MacOS
cp -fR $pathToBackendApps/applications/performRegionalEventSimulation/regionalWindField/database/historical_storm/* $pathApp/Contents/MacOS/Databases
mkdir  $pathApp/Contents/MacOS/applications/opensees
mkdir  $pathApp/Contents/MacOS/applications/dakota
mkdir  $pathApp/Contents/MacOS/Examples
mkdir  $pathApp/Contents/MacOS/Databases
cp     ../../R2DExamples/Examples.json $pathApp/Contents/MacOS/Examples
cp -fr $pathToOpenSees/* $pathApp/Contents/MacOS/applications/opensees
cp -fr $pathToDakota/* $pathApp/Contents/MacOS/applications/dakota
cp -fr $pathApp/../../Databases/* $pathApp/Contents/MacOS/Databases

mkdir $pathApp/Contents/MacOS/share
mkdir $pathApp/Contents/MacOS/lib
mkdir $pathApp/Contents/MacOS/lib/qgis
cp -fR $pathApp/../../../qgisplugin/mac/Install/lib/* $pathApp/Contents/Frameworks
cp -fR $pathApp/../../../qgisplugin/mac/qgis-deps-0.9/stage/lib/* $pathApp/Contents/Frameworks
cp -fR $pathApp/../../../qgisplugin/mac/Install/share/* $pathApp/Contents/MacOS/share
cp -fR $pathApp/../../../qgisplugin/mac/Install/qgis/* $pathApp/Contents/MacOS/lib/qgis

install_name_tool -change @loader_path/libz.1.2.11.dylib @rpath/libz.1.2.11.dylib ./R2D.app/Contents/MacOS/R2D

# remove unwanted stuff

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
			 "createEDP/userEDP"
			 "createEDP/userEDP"
			 "createEDP/userEDP_R"
			 "createEDP/standardEarthquakeEDP_R"			 
			)

for app in "${notWantedApp[@]}"
do
   echo "removing $app"
   rm -fr $pathApp/Contents/MacOS/applications/$app
done

find ./$APP_FILE -name __pycache__ -exec rm -rf {} +;

#
# now before we codesign and verify, check userID file exists
#

userID="../userID.sh"

if [ ! -f "$userID" ]; then
    
    echo "creating dmg $DMG_FILENAME"
    hdiutil create $DMG_FILENAME -fs HFS+ -srcfolder ./$APP_FILE -format UDZO -volname $APP_NAME

    echo "No password & credential file to continue with codesig and App store verification"
    exit
fi

exit

source $userID
echo $appleID

#
# create dmg
#


if [ "${DMG_METHOD}" == "NEW" ]; then
    
    #
    # mv app into empty folder for create-dmg to work
    # brew install create-dmg
    #

    echo "codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $APP_FILE"
    codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $APP_FILE    
    
    mkdir app
    mv $APP_FILE app
    
    # swoop
    #create-dmg \
	#  --volname "${APP_NAME}" \
	#  --background "../background/background1.png" \
	#  --window-pos 200 120 \
	#  --window-size 550 400 \
	#  --icon-size 150 \
	#  --icon "${APP_NAME}.app" 150 190 \
	#  --hide-extension "${APP_NAME}.app" \
	#  --app-drop-link 400 185 \
	#  "${DMG_FILENAME}" \
	#  "app"
    
    # vertical 
    #create-dmg \
	#  --volname "${APP_NAME}" \
	#  --background "../background/background2.png" \
	#  --window-pos 200 120 \
	#  --window-size 475 550 \
	#  --icon-size 150 \
	#  --icon "${APP_NAME}.app" 235 125 \
	#  --hide-extension "${APP_NAME}.app" \
	#  --app-drop-link 235 400 \
	#  "${DMG_FILENAME}" \
	#  "app"
    
    #horizontal
    ../macInstall/create-dmg \
	--volname "${APP_NAME}" \
	--background "../macInstall/background3.png" \
	--window-pos 200 120 \
	--window-size 600 350 \
	--no-internet-enable \
	--icon-size 125 \
	--icon "${APP_NAME}.app" 125 130 \
	--hide-extension "${APP_NAME}.app" \
	--app-drop-link 450 130 \
	--codesign $appleCredential \
	"${DMG_FILENAME}" \
	"app"
    
    #  --notarize $appleID $appleAppPassword \
	
    mv ./app/$APP_FILE ./
    rm -fr app

else

    echo "codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $APP_FILE"
    codesign --deep --force --verbose --options=runtime  --sign "$appleCredential" $APP_FILE
        
    echo "hdiutil create $DMG_FILENAME -fs HFS+ -srcfolder ./$APP_FILE -format UDZO -volname $APP_NAME"
    hdiutil create $DMG_FILENAME -fs HFS+ -srcfolder ./$APP_FILE -format UDZO -volname $APP_NAME

    echo "Issue: codesign --force --sign "$appleCredential" $DMG_FILENAME"
    codesign --force --sign "$appleCredential" $DMG_FILENAME
    
fi




#
# notorize , create zip file & send to apple
#

echo "Issue the following: " 
echo "xcrun notarytool submit ./$DMG_FILENAME --apple-id $appleID --password $appleAppPassword --team-id $appleCredential"
echo "xcrun notarytool log ID --apple-id $appleID --team-id $appleCredential  --password $appleAppPAssword"

#echo "https://appleid.apple.com/account/"
#echo "under security generate app specific password: $APP_NAME"
#echo "xcrun altool --notarize-app -u appleID -p appleAppPassword -f ./$APP_NAME.zip --primary-bundle-id altool"
#echo "returns id: ID"

echo "Finally staple the dmg"
echo "xcrun stapler staple \"$APP_NAME\" $DMG_FILENAME"
