@echo off
cd build
cmake -G "Visual Studio 17 2022" -A x64 ^
	-DCMAKE_EXPORT_COMPILE_COMMANDS=On ^
	-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
	-DPython_FIND_IMPLEMENTATIONS:STRING="" ^
	..
IF ERRORLEVEL 1 GOTO HAD_ERROR
cmake --build . --config Release
IF ERRORLEVEL 1 GOTO HAD_ERROR
GOTO NO_ERROR
:HAD_ERROR
echo "Error happend. Build incomplete"
GOTO REAL_END
:NO_ERROR
echo "Success!"
:REAL_END
cd ..

