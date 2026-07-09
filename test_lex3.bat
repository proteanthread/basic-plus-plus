@echo off
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
    set "VS_PATH=%%i"
)
set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"
call "%VCVARS%" x64
cd source
cl /TC /std:c17 /W3 /O2 /I. /I../sdl2/include /D_CRT_SECURE_NO_WARNINGS /Fe:test_iso.exe test_isolated.c lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c core/memory.c core/errors.c core/stringpool.c core/config_file.c core/security.c core/platform.c
.\test_iso.exe
