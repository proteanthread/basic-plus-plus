@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo BASIC++ 4.0.1 (Stable) -- Windows 11 Build (MSVC)
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

echo [INFO] Running nmake msvc...
nmake /f Makefile msvc

if %ERRORLEVEL% neq 0 (
    echo [ERROR] MSVC build failed.
    exit /b %ERRORLEVEL%
)

cd /d "%~dp0"

echo.
echo ========================================================
echo Build complete! Windows executable: basicpp.exe
echo ========================================================
echo.

:: Build legacy prototypes (disabled — build separately if needed)
:: echo [INFO] Building legacy prototypes...
::
:: cd /d "%~dp0"
::
:: cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:int-bas.exe int-bas.c
:: if %ERRORLEVEL% neq 0 (
::     echo [WARN] int-bas.exe build failed.
:: ) else (
::     echo [OK] int-bas.exe built.
:: )
::
:: cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:core-bas.exe core-bas.c
:: if %ERRORLEVEL% neq 0 (
::     echo [WARN] core-bas.exe build failed.
:: ) else (
::     echo [OK] core-bas.exe built.
:: )
::
:: cl /TC /W3 /O2 /D_CRT_SECURE_NO_WARNINGS /Fe:tinybasic.exe tinybasic.c
:: if %ERRORLEVEL% neq 0 (
::     echo [WARN] tinybasic.exe build failed.
:: ) else (
::     echo [OK] tinybasic.exe built.
:: )
::
:: Clean up MSVC intermediates in root
del /q *.obj 2>nul

echo.
echo ========================================================
echo All builds complete.
echo ========================================================
endlocal
