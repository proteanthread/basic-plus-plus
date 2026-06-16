@echo off
rem =====================================================================
rem  build-fd.bat -- Build BASIC++ on FreeDOS using OpenWatcom
rem =====================================================================
rem
rem  USAGE:
rem    build-fd          Build 16-bit (default, real mode)
rem    build-fd 16       Build 16-bit (explicit)
rem    build-fd 32       Build 32-bit (DOS/4GW protected mode)
rem    build-fd clean    Remove all .obj files
rem
rem  PREREQUISITES:
rem    - OpenWatcom installed (set WATCOM environment variable)
rem    - wmake available on PATH
rem    - Run from the BASIC++ project root directory
rem
rem  DEFAULT CONFIGURATION:
rem    - Dialect: Palo Alto Tiny BASIC (PATB)
rem    - Modules: STDLIB only (no USB, FujiNet, UPnP, JIT)
rem    - Memory:  512K conventional (slim pools)
rem
rem  To customize dialects, modules, or memory, edit source\makefd
rem  before running this script. See docs\FreeDOS_Compilation_Guide.md
rem  for complete instructions.
rem
rem =====================================================================

rem --- Check for OpenWatcom ---
if "%WATCOM%"=="" goto no_watcom

rem --- Parse argument ---
if "%1"=="clean" goto do_clean
if "%1"=="32" goto build_32
if "%1"=="16" goto build_16
if "%1"=="" goto build_16

echo.
echo ERROR: Unknown option "%1"
echo Usage: build-fd [16 / 32 / clean]
goto end

:build_16
echo.
echo =========================================
echo  BASIC++ FreeDOS Build (16-bit real mode)
echo  Default dialect: PATB
echo =========================================
echo.
cd source
wmake -f makefd BITS=16
cd ..
if exist basicpp.exe goto build_ok_16
if exist bpp.exe goto build_ok_16
echo.
echo [ERROR] Build failed. Check errors above.
goto end

:build_ok_16
echo.
echo =========================================
echo  Build complete: bpp.exe (16-bit)
echo =========================================
goto end

:build_32
echo.
echo =========================================
echo  BASIC++ FreeDOS Build (32-bit DOS/4GW)
echo  Default dialect: PATB
echo =========================================
echo.
cd source
wmake -f makefd BITS=32
cd ..
if exist basicpp.exe goto build_ok_32
if exist bpp.exe goto build_ok_32
echo.
echo [ERROR] Build failed. Check errors above.
goto end

:build_ok_32
echo.
echo =========================================
echo  Build complete: bpp.exe (32-bit DOS/4GW)
echo =========================================
goto end

:do_clean
echo.
echo Cleaning object files...
cd source
wmake -f makefd clean
cd ..
echo Done.
goto end

:no_watcom
echo.
echo ERROR: WATCOM environment variable not set.
echo.
echo Please install OpenWatcom and set the WATCOM variable:
echo   SET WATCOM=C:\WATCOM
echo   SET PATH=%%WATCOM%%\BINW;%%PATH%%
echo   SET INCLUDE=%%WATCOM%%\H
echo.
echo Or run OWSETENV.BAT from your OpenWatcom installation.
goto end

:end
