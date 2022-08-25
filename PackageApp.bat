@ECHO OFF

:: How to use this file:
:: 1. Copy this batch file into the folder one directory up from the build folder
:: 2. Check the paths below to ensure they match the locations of OpenSees, Dakota, backend, etc., on your system
:: 3. Run the batch file. The R2D executable should now be ready to go.

ECHO "Packaging R2D for Windows"

SET BATCHPATH=%~dp0

ECHO %BATCHPATH%

:: Set the system paths below

set OPENSEES=C:\Users\vagrant\Desktop\OpenSees3.3.0-x64.exe
set DAKOTA=C:\Users\vagrant\Desktop\OpenSRABuild>C:\Users\vagrant\Desktop\dakota-6.14.0.Windows.x64
set PYTHONDIST=C:\Users\vagrant\Desktop\python

:: Copy over dakota

mkdir %BATCHPATH%R2DTool\build\applications\dakota
xcopy /s /y %DAKOTA%\bin %BATCHPATH%R2DTool\build\applications\dakota\
xcopy /s /y %DAKOTA%\share %BATCHPATH%R2DTool\build\applications\dakota\
 
:: Copy over opensees

mkdir %BATCHPATH%R2DTool\build\applications\opensees
xcopy /s /y %OPENSEES% %BATCHPATH%R2DTool\build\applications\opensees\

:: Copy over the python distribution

mkdir %BATCHPATH%R2DTool\build\applications\python
xcopy /s /y %PYTHONDIST% %BATCHPATH%R2DTool\build\python\

ECHO "Done packaging R2D for Windows"
