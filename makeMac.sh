#!/bin/bash

QT=/Users/fmckenna/Qt/6.10.1/macos
PREFIX=/Users/fmckenna/NHERI/QGIS/DEPS

# Check for Conan (Added a space after ! for safety)                                                                                                                     
if ! command -v conan &> /dev/null; then
    echo "FAILED 'conan' command not found. Please install it first."
    return 1 2>/dev/null || exit 1
fi


# Check for CMakeLists.txt before doing anything
[ -f "CMakeLists.txt" ] || {
    echo "FAILED CMakeLists.txt not found in $(pwd)"
    return 1 2>/dev/null || exit 1
}

echo "Cleaning up old build directory..."
rm -fr build

export CMAKE_PREFIX_PATH=$PREFIX

echo "Running conan install..."
# copy conanfile2 to conanfile till i remove old qmake build, conan install and then copy back
if [ -f "conanfile2.py" ]; then
    mv conanfile.py conanfile.old
    cp conanfile2.py conanfile.py
fi

conan install . --build missing -s build_type=Release || {
    echo "FAILED: Conan install failed."
    return 1 2>/dev/null || exit 1
}    

if [ -f "conanfile2.py" ]; then
    mv conanfile.old conanfile.py
fi


# Use a subshell for the build steps
(

    mkdir -p build/Release
    cd build/Release || exit 1
    
    echo "Configuring CMake..."
    cmake -S ../.. \
      -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$QT;$PREFIX" \
      -DQt6Keychain_DIR=$DEPS \
      -DQWT_LIBRARY=$PREFIX/lib/libqwt.dylib \
      -DQWT_INCLUDE_DIR=$PREFIX/include/qwt \
      -DQCA_INCLUDE_DIR=$PREFIX/include/Qca-qt6/QtCrypto \
      -DQCA_LIBRARY=$PREFIX/lib/libqca-qt6.dylib \
      -DPython_EXECUTABLE=/Users/fmckenna/python_env/python3-qgis/bin/python3 \
      -DSIP_BUILD_EXECUTABLE=/Users/fmckenna/python_env/python3-qgis/bin/sip-build \
      -DQSCINTILLA_INCLUDE_DIR=$QT/include \
      -DQSCINTILLA_LIBRARY=$QT/lib/libqscintilla2_qt6.dylib \
    || {
	echo "FAILED: cmake CONFIGURATION failed."
	return 1 2>/dev/null || exit 1	
    }


    echo "Building with 8 cores..."    
    cmake --build . --parallel 8 || {
	echo "FAILED: cmake BUILD failed."
	return 1 2>/dev/null || exit 1	
    }

    # and now copy files and exe path changes to get it to run!
    mkdir ./R2D.app/Contents/PlugIns
    mkdir ./R2D.app/Contents/Frameworks
    cp    $PREFIX/lib/libqwt.6.dylib  ./R2D.app/Contents/Frameworks
    xcrun install_name_tool -change libqwt.6.dylib @rpath/libqwt.6.dylib ./R2D.app/Contents/MacOS/R2D
    cp -r ../../../QGIS/build/output/Contents/PlugIns/* ./R2D.app/Contents/PlugIns
    mkdir -p ./R2D.app/Contents/PlugIns/crypto
    cp    $PREFIX/lib/qca-qt6/crypto/libqca-*.dylib ./R2D.app/Contents/PlugIns/crypto/
    install_name_tool -add_rpath @executable_path/../Frameworks ./R2D.app/Contents/MacOS/R2D
)


