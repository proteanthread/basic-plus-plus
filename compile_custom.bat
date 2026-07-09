@echo off
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
    set "VS_PATH=%%i"
)
set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"
call "%VCVARS%" x64
cd /d "%~dp0source"
echo Compiling...
cl /TC /std:c17 /W3 /O2 /I. /I../sdl2/include /D_CRT_SECURE_NO_WARNINGS /DINPUT_CONSOLE /MP /c $(CORE_SOURCES) $(LEXER_SOURCES) $(PARSER_SOURCES) \
echo Linking...
link /OUT:basicpp-console.exe /SUBSYSTEM:CONSOLE *.obj advapi32.lib ws2_32.lib /LIBPATH:../sdl2/lib/x64 SDL2.lib SDL2main.lib Shell32.lib User32.lib Gdi32.lib Winmm.lib
echo Moving...
move /Y basicpp-console.exe ..\basicpp-console.exe
