@echo off
setlocal enabledelayedexpansion

:: 1. Initialize the Visual Studio Environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

set "QT=C:/Qt6/6.10.2/msvc2022_64"
set "BASE_DIR=%~dp0"
:: Ensure this path is correct relative to the script
set "PREFIX=%BASE_DIR%..\QGIS\DEPS"

:: 2. Check for Conan
where conan >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo FAIL: 'conan' command not found.
    pause
    exit /b 1
)

if exist "build\Release" rd /s /q "build\Release"

echo Running conan install...

:: 3. Swap conanfiles if conanfile2 exists
if exist "conanfile2.py" (
    if exist "conanfile.py" move /y conanfile.py conanfile.old >nul
    copy /y conanfile2.py conanfile.py >nul
)

conan install . --build=missing -s build_type=Release ^
  -c tools.cmake.cmaketoolchain:generator=Ninja ^
  -c tools.cmake.cmaketoolchain:toolset_arch=None

if %ERRORLEVEL% neq 0 (
    echo FAIL: Conan install failed.
    if exist "conanfile.old" move /y conanfile.old conanfile.py >nul
    pause
    exit /b 1
)

:: FIXED: Changed { } to ( )
if exist "conanfile.old" (
   move /y conanfile.old conanfile.py >nul
)

:: Create and enter build directory
if not exist "build\Release" mkdir "build\Release"
cd build\Release

if exist CMakeCache.txt del /q CMakeCache.txt
if exist CMakeFiles rd /s /q CMakeFiles

echo Configuring CMake...
:: NOMINMAX is included here to fix the 'min/max' errors in QGIS headers
cmake ../.. ^
    -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS="/DNOMINMAX" ^
    -DQt6_DIR="%QT%/lib/cmake/Qt6" ^
    -DCMAKE_PREFIX_PATH="%QT%;%PREFIX%" ^
    -DQt6Keychain_DIR="%PREFIX%" ^
    -DQWT_LIBRARY="%PREFIX%/lib/qwt.lib" ^
    -DQWT_INCLUDE_DIR="%PREFIX%/include/qwt" ^
    -DQCA_INCLUDE_DIR="%PREFIX%/include/Qca-qt6/QtCrypto" ^
    -DQCA_LIBRARY="%PREFIX%/lib/qca-qt6.lib" ^
    -DPython_EXECUTABLE="%PYTHON_EXE%" ^
    -DQSCINTILLA_INCLUDE_DIR="%PREFIX%/include" ^
    -DQSCINTILLA_LIBRARY="%PREFIX%/lib/qscintilla2_qt6.lib"

if %ERRORLEVEL% neq 0 (
    echo FAIL: CMake configuration failed.
    cd ../..
    pause
    exit /b 1
)

echo Building...
cmake --build . --parallel %NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo FAIL: Build failed.
    cd ../..
    pause
    exit /b 1
)

echo Build complete!

:: Deployment (Ensure these paths exist)
if exist "EE_UQ.exe" (
    copy /y EE_UQ.exe ..\EE-UQ_Windows_Download
    cd ..\WW-UQ_Windows_Download
    windeployqt EE_UQ.exe
)

:: 4. Cleanup/Examples (Fixed 'rm' which is not a CMD command)
echo Copying Examples folder...
xcopy /E /I /Y "%BASE_DIR%Examples" ".\"

if exist ".\Examples\.aurore" rd /s /q ".\Examples\.aurore"
if exist ".\Examples\.gitignore" del /q ".\Examples\.gitignore"

cd ../..
echo All tasks complete!
pause


exit \b 1


@echo on
setlocal enabledelayedexpansion

:: 1. Initialize the Visual Studio Environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

set "QT=C:/Qt6/6.10.2/msvc2022_64"
set "BASE_DIR=%~dp0"
set "PREFIX=%BASE_DIR%..\QGIS\DEPS"


:: 2. Check for Conan
where conan >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo FAIL: 'conan' command not found.
    pause
    exit /b 1
)

if exist "build\Release" rd /s /q "build\Release"


echo Running conan install...

:: 3. Swap conanfiles if conanfile2 exists
if exist "conanfile2.py" (
    if exist "conanfile.py" move /y conanfile.py conanfile.old >nul
    copy /y conanfile2.py conanfile.py >nul
)

conan install . --build=missing -s build_type=Release ^
  -c tools.cmake.cmaketoolchain:generator=Ninja ^
  -c tools.cmake.cmaketoolchain:toolset_arch=None
echo "HELL2"
if %ERRORLEVEL% neq 0 (
    echo FAIL: Conan install failed.
    if exist "conanfile.old" move /y conanfile.old conanfile.py >nul
    pause
    exit /b 1
)
echo "HELLO3"
:: Restore original conanfile
if exist "conanfile.old" {
   move /y conanfile.old conanfile.py >nul
}

echo "HELLO"


cd build/Release
if exist CMakeCache.txt del /q CMakeCache.txt
if exist CMakeFiles rd /s /q CMakeFiles

echo Configuring CMake...
set "CMAKE_PREFIX_PATH=%QT%"

:: We use forward slashes for CMake paths to avoid escape character issues
call cmake ../.. ^
    -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS="/DNOMINMAX" ^
    -DQt6_DIR="%QT%/lib/cmake/Qt6" ^
    -DCMAKE_PREFIX_PATH="%QT%;%PREFIX%" ^
    -DQt6Keychain_DIR="%PREFIX%" ^
    -DQWT_LIBRARY="%PREFIX%/lib/qwt.lib" ^
    -DQWT_INCLUDE_DIR="%PREFIX%/include/qwt" ^
    -DQCA_INCLUDE_DIR="%PREFIX%/include/Qca-qt6/QtCrypto" ^
    -DQCA_LIBRARY="%PREFIX%/lib/qca-qt6.lib" ^
    -DPython_EXECUTABLE="%PYTHON_EXE%" ^
    -DQSCINTILLA_INCLUDE_DIR="%PREFIX%/include" ^
    -DQSCINTILLA_LIBRARY="%PREFIX%/lib/qscintilla2_qt6.lib" ^

if %ERRORLEVEL% neq 0 (
    echo FAIL: CMake configuration failed.
    cd ..
    pause
    exit /b 1
)

:: 5. Build with all available cores
echo Building...
call cmake --build . --parallel %NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo FAIL: Build failed.
    cd ..
    pause
    exit /b 1
)

echo Build complete!

copy EE_UQ.exe ..\EE-UQ_Windows_Download
cd ..\WW-UQ_Windows_Download
windeployqt EE_UQ.exe

:: 4. Copy Examples (Adjusted for Windows folder structure)
:: On Windows, your EXE is likely in build/Release/bin or just build/Release/
echo Copying Examples folder...
xcopy /E /I /Y ".\Examples" ".\"

:: Remove unwanted hidden files/folders (Windows equivalent of rm -rf)
rem if exist ".\Examples\.archive" rd /s /q ".\Examples\.archive"
rm if exist ".\Examples\.aurore" rd /s /q ".\Examples\.aurore"
rm if exist ".\Examples\.gitignore" del /q ".\Examples\.gitignore"

cd ../..
echo Build complete!
pause