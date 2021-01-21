#!/bin/bash 

# Written by Stevan Gavrilovic, University of California, Berkeley
# Usage: 
# 	1) cd the build folder containing the .app file of your application
# 	2) Copy this script file over to the build folder
#   3) Set the path to macdeployqt on your system and change the AppName to match the *.app file below
# 	3) Run: bash PackageApp.sh

# ********* THINGS TO CHANGE START *********
# Set the path to your macdeployqt here
pathMacDepQt="/Users/steve/Qt/5.15.1/clang_64/bin/macdeployqt"

# Name of the application
AppName="R2D"
# ********* THINGS TO CHANGE END *********

appFile=$AppName".app"
appdmg=$AppName".dmg"

# Check to see if the required macdeployqt file exists at the given location
if [ ! -f "$pathMacDepQt" ]; then   
	
	echo "Could not find macdeployqt at $pathMacDepQt. Exiting"
	exit
fi


# Get the path to the app file
pathApp=`pwd`/$appFile

# Check to see if the RDT app file exists in the current directory
if ! [ -x "$(command -v open $pathApp)" ]; then
	echo "$appFile does not exist. Exiting."
	exit 
fi

# Variable to the path to the dmg
pathdmg="`pwd`/$appdmg"

# Remove the old dmg file if it already exists
if [ -f "$pathdmg" ]; then   
	
	rm $pathdmg	
fi

# Run the macdeployqt to create the dmg
$pathMacDepQt $pathApp -dmg

# Check to see if the dmg was created and exists
if [ ! -f "$pathdmg" ]; then   
	
	echo "Could not find the .dmg file at $appdmg. Exiting"
	
	exit
fi

# Convert the DMG into a new dmg with read/write mode enabled
pathRWdmg="`pwd`/RW_New.dmg"

hdiutil convert -format UDRW -o "`pwd`/RW_New.dmg" "$pathdmg" -ov

# Check to see if the new writeable dmg was created and exists
if [ ! -f "$pathRWdmg" ]; then   
	
	echo "Could not find the new .dmg file that should have been created at $pathRWdmg. Exiting"
	
	exit
fi


# use process redirection to capture the mount point and dev entry
IFS=$'\n' read -rd '\n' mount_point < <(
    
	# mount the diskimage
    hdiutil attach -plist "$pathRWdmg" | \

    # extract mount point entry
	xpath '(//key[.="mount-point"]/following-sibling::string[1]/node())'
	
)
	
IFS=$'\n' read -rd '\n' dev_entry < <(
	# mount the diskimage
	hdiutil attach -plist "$pathRWdmg" | \

	# extract mount point and dev entry
	xpath '(//key[.="dev-entry"]/following-sibling::string[1]/node())[2]'

)
		
echo $mount_point
echo $dev_entry
	
# Define the paths to the application and to libEsriCommonQt.dylib - this should not change
pathAppBin=$mount_point/$appFile/Contents/MacOS/RDT

pathAppLib=$mount_point/$appFile/Contents/Frameworks/libEsriCommonQt.dylib

# Get the paths that are in the libraries - these paths will be changed to relative paths instead of the absolute paths
pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

# echo $pathEsriCommonQt
# echo $pathLibruntimecore

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

echo "Path substitution complete!"

# Detach the dmg 
hdiutil detach "$dev_entry"

# Convert the dmg back to read only - overwriting the original
hdiutil convert -format UDRO -o "$pathdmg" "$pathRWdmg" -ov

# Remove the writable dmg
rm $pathRWdmg

echo "Done!"
