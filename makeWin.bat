@echo off
setlocal enabledelayedexpansion

:: 1. Initialize the Visual Studio Environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

set "QT=C:/Qt6/6.10.2/msvc2022_64"
set "BASE_DIR=%~dp0"
:: Ensure this path is correct relative to the script
set "QGIS_DEPS=%BASE_DIR%..\QGIS\DEPS"
set "QGIS_DIR=%BASE_DIR%..\QGIS\build\output\bin"
set "OUTPUT_DIR=%BASE_DIR%\build\output"

:: 2. Check for Conan
where conan >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo FAIL: 'conan' command not found.
    pause
    exit /b 1
)

if exist "build\Release" rd /s /q "build\Release"
#if exist "%OUTPUT_DIR%" rd /s /q "$OUTPUT_DIR"

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
    -DCMAKE_PREFIX_PATH="%QT%;%QGIS_DEPS%" ^
    -DQt6Keychain_DIR="%PREFIX%" ^
    -DQWT_LIBRARY="%QGIS_DEPS%/lib/qwt.lib" ^
    -DQWT_INCLUDE_DIR="%QGIS_DEPS%/include/qwt" ^
    -DQCA_INCLUDE_DIR="%QGIS_DEPS%/include/Qca-qt6/QtCrypto" ^
    -DQCA_LIBRARY="%QGIS_DEPS%/lib/qca-qt6.lib" ^
    -DPython_EXECUTABLE="%PYTHON_EXE%" ^
    -DQSCINTILLA_INCLUDE_DIR="%QGIS_DEPS%/include" ^
    -DQSCINTILLA_LIBRARY="%QGIS_DEPS%/lib/qscintilla2_qt6.lib"

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


rem if exist "R2D.exe" (
rem     if not exist "%OUTPUT_DIR" mkdir "%OUTPUT_DIR%"
rem     copy /y R2D.exe "%OUTPUT_DIR%
rem     robocopy "%QGIS_DIR%" "%OUTPUT_DIR%" /E /COPYALL /R:2 /W:
rem     cd "%OUTPUT_DIR%
rem     windeployqt R2D.exe    
rem )

rem cd ../..
echo "All TASKS COMPLETE  R2D in %OUTPUT_DIR"


