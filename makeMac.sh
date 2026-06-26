#!/bin/bash
set -euo pipefail

QT=/Users/fmckenna/Qt/6.10.1/macos
#PREFIX=/Users/fmckenna/NHERI/QGIS/DEPS

PROJECT_NAME="R2D"

CONAN_PROFILE="default"
BUILD_DIR="build"
ARCH=""
PREFIX=$(pwd)/../QGIS/DEPS
RELEASE="NO"
RELEASE_FLAG="-DNO_RELEASE"
APPLICATIONS_DIR=$(pwd)/../SimCenterBackendApplications/applications
PYTHON_DIR="${HOME}/cpython"
WIPE="FALSE"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --arch|-a)
            ARCH="$2"
            if [[ "$ARCH" != "x86_64" && "$ARCH" != "arm64" ]]; then
                echo "Error: Invalid architecture '$ARCH'."
                echo "Supported architectures: x86_64, arm64"
                exit 1
            fi

            CONAN_PROFILE="macos-$ARCH"
            BUILD_DIR="build_$ARCH"
            # PREFIX="${PREFIX}_$ARCH"
            APPLICATIONS_DIR="${APPLICATIONS_DIR}_$ARCH"	    

            if [[ "$ARCH" == "x86_64" ]]; then
		pathToOpenSees="${HOME}/bin/OpenSeesLatest_x86_64"
		pathToDakota="${HOME}/dakota/dakota-6.19.0"
		PYTHON_DIR="${PYTHON_DIR}/python_x86_64"		
	    else
		pathToOpenSees="${HOME}/bin/OpenSeesLatest_arm64"
		pathToDakota="${HOME}/dakota/dakota-6.22.0"
		PYTHON_DIR="${PYTHON_DIR}/python_arm64"				
            fi	    
	    
            shift 2
            ;;
	--release|-r)
	    RELEASE="YES"
	    RELEASE_FLAG="-D_SC_RELEASE"	    
	    shift 1
	    ;;
	--wipe|-w)
	    WIPE="TRUE"
	    shift 1
	    ;;
        *)	    
            echo "Unknown argument: $1"
            echo "Usage: $0 --arch <arm64|x86_64> <--release> <--wipe>"
            exit 1
            ;;
    esac
done

echo "Architecture: ${ARCH}"
echo "Conan profile : ${CONAN_PROFILE}"
echo "Build directory: ${BUILD_DIR}"
echo "PREFIX: ${PREFIX}"
echo ""

# Define some helpers to print messages
die() {
    echo ""    
    echo "$1"
    echo ""    
    exit 1
}

msg() {
    echo ""
    echo "$1"
    echo ""
}

#
# Check for Conan .. 
#

if ! command -v conan &> /dev/null; then
    echo "FAILED 'conan' command not found. Please install it first."
    return 1 2>/dev/null || exit 1
fi

#
# Remove old build dir
#

if [[ "$WIPE" == "TRUE" ]]; then
    if [[ "$BUILD_DIR" != "$HOME" ]]; then
	echo "Cleaning up old build directory..."
	rm -rf "${BUILD_DIR}"
    fi
fi

export CMAKE_PREFIX_PATH=$PREFIX

#
# Conan build dependencies
#

msg "Running conan install..."
 
# copy conanfile2 to conanfile till i remove old qmake build
if [ -f "conanfile2.py" ]; then
    mv conanfile.py conanfile.old
    cp conanfile2.py conanfile.py
fi

conan install . --output-folder="${BUILD_DIR}" --build missing -s build_type=Release -pr "${CONAN_PROFILE}" || die "FAIL : Conan install failed."

if [ -f "conanfile2.py" ]; then
    mv conanfile.old conanfile.py
fi

#
# Configure CMake
#

CMAKE_ARCH_FLAG=""
if [ -n "${ARCH}" ]; then
    CMAKE_ARCH_FLAG="-DCMAKE_OSX_ARCHITECTURES=${ARCH}"
fi

echo "Configuring CMake..."
set +x
cmake -B "${BUILD_DIR}" -S . \
      -DCMAKE_TOOLCHAIN_FILE="${BUILD_DIR}/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release ${CMAKE_ARCH_FLAG} \
      -DCMAKE_CXX_FLAGS=${RELEASE_FLAG} \
      -DCMAKE_PREFIX_PATH="$QT;$PREFIX" \
      -DQt6Keychain_DIR=$PREFIX \
      -DQWT_LIBRARY=$PREFIX/lib/libqwt.dylib \
      -DQWT_INCLUDE_DIR=$PREFIX/include/qwt \
      -DQCA_INCLUDE_DIR=$PREFIX/include/Qca-qt6/QtCrypto \
      -DQCA_LIBRARY=$PREFIX/lib/libqca-qt6.dylib \
      -DPython_EXECUTABLE=/Users/fmckenna/python_env/python3-qgis/bin/python3 \
      -DSIP_BUILD_EXECUTABLE=/Users/fmckenna/python_env/python3-qgis/bin/sip-build \
      -DQSCINTILLA_INCLUDE_DIR=$QT/include \
      -DQSCINTILLA_LIBRARY=$QT/lib/libqscintilla2_qt6.dylib || die "FAIL: CMake configure failed."

set -x

#
# in case no wipe, removing app and touchingh WorkflowApp and main
#

touch WorkflowAppPBE.cpp main.cpp
rm -fr "${BUILD_DIR}/${PROJECT_NAME}.app"

#
# Now build With CMake
#

msg "Building with 8 cores..."
cmake --build "${BUILD_DIR}" --parallel 8 || die "FAIL: CMake build failed."


APP_NAME="${PROJECT_NAME}.app"
APP_DIR="${BUILD_DIR}/${APP_NAME}"

# and now copy files and exe path changes to get it to run!
#cd "${BUILD_DIR}"

mkdir "${APP_DIR}/Contents/PlugIns"
mkdir "${APP_DIR}/Contents/Frameworks"
cp    "$PREFIX/lib/libqwt.6.dylib"  "${APP_DIR}/Contents/Frameworks"
xcrun install_name_tool -change libqwt.6.dylib @rpath/libqwt.6.dylib "${APP_DIR}/Contents/MacOS/R2D"
cp -r ../QGIS/build/output/Contents/PlugIns/* "${APP_DIR}/Contents/PlugIns"
mkdir -p "${APP_DIR}/Contents/PlugIns/crypto"
cp    $PREFIX/lib/qca-qt6/crypto/libqca-*.dylib "${APP_DIR}/Contents/PlugIns/crypto/"
install_name_tool -add_rpath @executable_path/../Frameworks "${APP_DIR}/Contents/MacOS/R2D"

#
# macdeploy the application
# 

macdeployqt "${APP_DIR}"

#
# Copy Examples, Databases and Applications now to the app folder
#

mkdir "${APP_DIR}/Contents/MacOS/Examples"
mkdir "${APP_DIR}/Contents/MacOS/Databases"
mkdir "${APP_DIR}/Contents/MacOS/applications"
mkdir "${APP_DIR}/Contents/MacOS/applications/opensees"
mkdir "${APP_DIR}/Contents/MacOS/applications/dakota"

cp     ../R2DExamples/Examples.json "$APP_DIR/Contents/MacOS/Examples"
cp -fR "${APPLICATIONS_DIR}/performRegionalEventSimulation/regionalWindField/database/historical_storm/"* "${APP_DIR}/Contents/MacOS/Databases"
cp -fr "${APPLICATIONS_DIR}"/* "${APP_DIR}/Contents/MacOS/applications"
cp -fr "${pathToOpenSees}"/* "./${APP_DIR}/Contents/MacOS/applications/opensees"
cp -fr "${pathToDakota}"/*   "./${APP_DIR}/Contents/MacOS/applications/dakota"

# Check if file exists
UNWANTED_FILE=unwanted_apps.txt
if [[ ! -f "$UNWANTED_FILE" ]]; then
    echo "Error: $UNWANTED_FILE not found!"
    exit 1
fi

# Read file line by line and remove each app
echo "Removing unwanted: "
while IFS= read -r app || [[ -n "$app" ]]; do
    # Skip empty lines and comments
    [[ -z "$app" || "$app" =~ ^# ]] && continue
    
    echo "Removing $app: ./$APP_DIR/Contents/MacOS/applications/$app"
    rm -fr "./$APP_DIR/Contents/MacOS/applications/$app"

done < "$UNWANTED_FILE"


if [[ "$RELEASE" != "YES" ]]; then
    die "Build complete!"
fi

# ===============================
# Release Build
# ===============================

msg "Now Doing Stuff for a Release"

#
# Clean up any __pyacache_ files
#

find "${APP_DIR}" -name __pycache__ -exec rm -rf {} +

msg "Cleanup complete!"

userID="userID.sh"

if [ ! -f "$userID" ]; then
    echo "No password & credential file to continue with codesig and App store verification .. done"
    exit 1
else
    source userID.sh    
fi

#
# now codesign
#

msg "codesigning ${APP_DIR} --deep"    
codesign --force --deep --verbose --timestamp --options=runtime --sign "$appleCredential" "$APP_DIR" || die "FAIL: codesign failed."

#
# Sign QtWebEngineProcess with its bundled entitlements as --deep on APP overwrites this, making plots appear empty in notarized builds
#

WEBENGINE_PROCESS="${APP_DIR}/Contents/Frameworks/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app"
msg "codesigning ${WEBENGINE_PROCESS}"
WEBENGINE_ENTITLEMENTS="${WEBENGINE_PROCESS}/Contents/Resources/QtWebEngineProcess.entitlements"
if [ -f "${WEBENGINE_ENTITLEMENTS}" ]; then
    codesign --force --timestamp --options=runtime \
             --entitlements "${WEBENGINE_ENTITLEMENTS}" \
             --sign "$appleCredential" "${WEBENGINE_PROCESS}" || die "FAIL: QtWebEngineProcess codesign failed."
fi

#
# build dmg bundle
#

DMG_METHOD="NEW"
if [ "${DMG_METHOD}" = "NEW" ]; then

    (
        cd "${BUILD_DIR}"
        mkdir app
        mv "${APP_NAME}" app
        set -x

        ../macInstall/create-dmg \
            --volname "${PROJECT_NAME}" \
            --background "../macInstall/background3.png" \
            --window-pos 200 120 \
            --window-size 600 350 \
            --no-internet-enable \
            --icon-size 125 \
            --icon "${APP_NAME}" 125 130 \
            --hide-extension "${APP_NAME}" \
            --app-drop-link 450 130 \
            --codesign "$appleCredential" \
            "R2D_Mac_Download_${ARCH}.dmg" \
            "app"

        set +x
        mv "./app/${APP_NAME}" ./
        rm -fr app
    )

else

    (
        cd "${BUILD_DIR}"
        hdiutil create "R2D_Mac_Download_${ARCH}.dmg" -fs HFS+ -srcfolder "./${APP_NAME}" -format UDZO -volname "${APP_NAME}"
    )

fi


#
# now xcrun stuff to get it signed and stapled
#

msg "DMG built .. now uploading to Apple for validation"

xcrun notarytool submit "${BUILD_DIR}/R2D_Mac_Download_${ARCH}.dmg" --apple-id "$appleID" --password "$appleAppPassword" --team-id "$appleCredential"

msg "Manual Mode! ... "
echo ""
echo "last command returns id: ID,  wait awhile for apple to verify then check status & log with: "
echo "   .. info for status and if succesull staple, otherwise look at log file and correct!"
echo "xcrun notarytool info ID  --apple-id $appleID --password $appleAppPassword --team-id $appleCredential"
echo "xcrun notarytool log ID --apple-id $appleID --password $appleAppPassword --team-id $appleCredential"
echo ""
echo "Finally staple the dmg"
echo "xcrun stapler staple  ${BUILD_DIR}/R2D_Mac_Download_${ARCH}.dmg"


echo "Release Build complete!"

