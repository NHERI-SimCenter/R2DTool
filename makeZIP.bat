
REM ## make the exe .. it assumes EE-UUQ_Windows_Download exists with vcredist and openssl there

cd build
conan install .. --build missing
qmake ..\R2D.pro
nmake

REM ## copy application into folder and winddeployqt it

copy .\R2D.exe .\R2D_Windows_Download
cd R2D_Windows_Download
windeployqt R2D.exe

REM ## copy examples.json from R2DExamples

rmdir /s /q .\Examples
mkdir .\Examples
copy  ..\..\..\R2DExamples\Examples.json .\Examples

REM ## delete applications folder and copy new stuff

rmdir /s /q .\applications
mkdir .\applications
xcopy /s /e ..\..\..\SimCenterBackendApplications\applications  .\applications







