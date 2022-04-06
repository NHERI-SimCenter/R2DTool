@ECHO OFF

:: How to use this file:
:: 1. Create a folder where you want to build R2D
:: 2. Copy this batch file into the folder. 
:: 3. Copy your R2DUserPass.h file into the same folder as this batch file, only if you have one.
:: 4. Check the paths below to ensure they match the locations of Qt, Python, MSVC, etc., on your system
:: 5. Run the batch file. The R2D executable will be in the release folder. 

ECHO "Starting Build of R2D for Windows"

SET BATCHPATH=%~dp0

ECHO %BATCHPATH%

:: Set the system variables below

set PYTHON=C:\PYTHON38-x64
set PYTHONNET_PYDLL=%PYTHON%\python3.8.dll
set QT=C:\Qt\5.15.2\msvc2019_64\bin
set PATH=%PYTHON%;%PYTHON%\Scripts;%QT%;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
pip.exe install conan
conan user
conan profile new default --detect
conan profile show default
conan profile update settings.compiler="Visual Studio" default
conan profile update settings.compiler.version="16" default
conan remote add simcenter https://nherisimcenter.jfrog.io/artifactory/api/conan/simcenter
echo %PATH%

:: Check if python exists

python -V || exit /b

:: Clone the repos

cd %BATCHPATH%

git clone https://github.com/NHERI-SimCenter/SimCenterBackendApplications.git
git clone https://github.com/NHERI-SimCenter/SimCenterCommon.git
git clone https://github.com/NHERI-SimCenter/R2DTool.git
git clone https://github.com/NHERI-SimCenter/R2DExamples.git
git clone https://github.com/sgavrilovic/QGISPlugin.git

:: Update the repos if they already exist

cd QGISPlugin
git pull
cd ..

cd R2DExamples
git pull
cd ..

cd R2DTool
git pull
cd ..

cd SimCenterBackendApplications
git pull
cd ..

cd SimCenterCommon
git pull
cd ..

dir

:: Build the backend

cd SimCenterBackendApplications
mkdir build
cd build
conan install .. --build missing
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
cmake --install .
cd ..
cd ..

:: Build R2D

:: Copy over the R2D User pass file

copy R2DUserPass.h %CD%\R2DTool\

cd R2DTool
mkdir build
cd build
conan install .. --build missing
qmake ..\R2D.pro
set CL=/MP
nmake
cd ..

cd ..

:: Run windeployqt to copy over the qt dlls

%QT%\windeployqt.exe %BATCHPATH%\R2DTool\build

:: Copy over the QGIS files

xcopy /s /y %BATCHPATH%\QGISPlugin\win\DLLs %BATCHPATH%R2DTool\build\
 
:: Copy over the QGIS plugins 

xcopy /s /y %BATCHPATH%QGISPlugin\mac\Install\share\qgis %BATCHPATH%R2DTool\build\

:: Copy over the example file

mkdir %BATCHPATH%R2DTool\build\Examples

copy %BATCHPATH%R2DExamples\Examples.json %CD%\R2DTool\build\Examples\

:: Copy over the applications folder

mkdir %BATCHPATH%R2DTool\build\applications

xcopy /s /y %BATCHPATH%SimCenterBackendApplications\applications %BATCHPATH%R2DTool\build\applications\

ECHO "Done Building R2D for Windows"

ECHO "You can find R2D.exe in "%BATCHPATH%R2DTool\build