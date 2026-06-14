@echo off
echo =========================================
echo Building BASIC++ for Windows 11 (MSVC)
echo =========================================

cd source
nmake /f Makefile msvc
if %ERRORLEVEL% neq 0 (
    echo [ERROR] MSVC build failed.
    exit /b %ERRORLEVEL%
)
cd ..

echo.
echo =========================================
echo Building BASIC++ for FreeDOS (Watcom)
echo =========================================

cd source
wmake -f Makefile watcom
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Watcom build failed.
    exit /b %ERRORLEVEL%
)
cd ..

echo.
echo Builds complete!
echo Windows executable: basicpp.exe
echo FreeDOS executable: bpp.exe
