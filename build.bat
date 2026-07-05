@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo BASIC++ 4.1.1 (Stable) -- Windows 11 Build (MSVC)
echo ========================================================

:: Attempt to locate Visual Studio using vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe not found. Is Visual Studio installed?
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
    echo [ERROR] Could not locate a Visual Studio installation.
    exit /b 1
)

set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "%VCVARS%" (
    echo [ERROR] vcvarsall.bat not found at "%VCVARS%".
    exit /b 1
)

:: Set up the MSVC environment for x64 build
echo [INFO] Setting up Visual Studio Environment from:
echo        %VS_PATH%
call "%VCVARS%" x64

:: Navigate to the source directory and run nmake
cd /d "%~dp0source"

if "%1"=="lite" (
    echo [INFO] Running nmake msvc-lite...
    nmake /f Makefile msvc-lite
    if !ERRORLEVEL! neq 0 (
        echo [ERROR] MSVC Lite build failed.
        exit /b !ERRORLEVEL!
    )
) else (
    echo [INFO] Running nmake msvc...
    nmake /f Makefile msvc
    if !ERRORLEVEL! neq 0 (
        echo [ERROR] MSVC Standard build failed.
        exit /b !ERRORLEVEL!
    )
)

cd /d "%~dp0"

echo.
echo ========================================================
echo Build complete! Windows executable: basicpp.exe
echo ========================================================
echo.

:: Build legacy prototypes
echo [INFO] Building legacy prototypes...

cd /d "%~dp0"

cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:apple2.exe standalone\source\apple2.c
if %ERRORLEVEL% neq 0 (
    echo [WARN] apple2.exe build failed.
) else (
    echo [OK] apple2.exe built.
)

cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:level1.exe standalone\source\level1.c
if %ERRORLEVEL% neq 0 (
    echo [WARN] level1.exe build failed.
) else (
    echo [OK] level1.exe built.
)

cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:tinybasic.exe standalone\source\tinybasic.c
if %ERRORLEVEL% neq 0 (
    echo [WARN] tinybasic.exe build failed.
) else (
    echo [OK] tinybasic.exe built.
)

cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:1964.exe standalone\source\1964.c
if %ERRORLEVEL% neq 0 (
    echo [WARN] 1964.exe build failed.
) else (
    echo [OK] 1964.exe built.
)

:: Build trans.exe and bppc.exe
echo [INFO] Building trans.exe and bppc.exe...
cd /d "%~dp0source"

nmake /f Makefile trans
if %ERRORLEVEL% neq 0 (
    echo [WARN] trans.exe build failed.
) else (
    echo [OK] trans.exe built.
    move /Y trans.exe ..\trans.exe >nul
)

nmake /f Makefile bppc
if %ERRORLEVEL% neq 0 (
    echo [WARN] bppc.exe build failed.
) else (
    echo [OK] bppc.exe built.
    move /Y bppc.exe ..\bppc.exe >nul
)

cd /d "%~dp0"
del /q source\*.obj 2>nul
del /q *.obj 2>nul

echo.
echo ========================================================
echo All builds complete.
echo ========================================================
endlocal
