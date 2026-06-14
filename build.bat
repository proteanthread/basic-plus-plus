@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo Building BASIC++ for Windows 11 (MSVC)
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
cd /d "C:\Users\rtdos\GitHub\basic-plus-plus\source"

echo [INFO] Running nmake...
nmake /f Makefile msvc

if %ERRORLEVEL% neq 0 (
    echo [ERROR] MSVC build failed.
    exit /b %ERRORLEVEL%
)

:: Copy the resulting executable to the root directory
if exist "basicpp.exe" (
    copy /Y basicpp.exe ..\basicpp.exe
)

cd ..

echo.
echo ========================================================
echo Build complete! Windows executable: basicpp.exe
echo ========================================================
endlocal
