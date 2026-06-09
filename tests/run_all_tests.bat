@echo off
REM =====================================================
REM  RUN_ALL_TESTS.BAT - Run all dialect test suites
REM =====================================================
REM  Runs each test_*.bas file with its required dialect.
REM  Reports PASS/FAIL for each.
REM =====================================================

setlocal enabledelayedexpansion
set PASS=0
set FAIL=0
set TOTAL=0

echo.
echo ========================================
echo  BASIC++ Dialect Test Runner
echo ========================================
echo.

REM --- PATB (default dialect) ---
set /a TOTAL+=1
echo [%TOTAL%] Testing PATB (Palo Alto Tiny BASIC)...
echo LOAD "tests/test_patb.bas"|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- GWBS ---
set /a TOTAL+=1
echo [%TOTAL%] Testing GWBS (GW-BASIC)...
(echo DIALECT "GWBS"& echo LOAD "tests/test_gwbs.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- QBAS ---
set /a TOTAL+=1
echo [%TOTAL%] Testing QBAS (QBasic)...
(echo DIALECT "QBAS"& echo LOAD "tests/test_qbas.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- EC55 ---
set /a TOTAL+=1
echo [%TOTAL%] Testing EC55 (ECMA-55 Minimal BASIC)...
(echo DIALECT "EC55"& echo LOAD "tests/test_ec55.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- E116 ---
set /a TOTAL+=1
echo [%TOTAL%] Testing E116 (ECMA-116 Full BASIC)...
(echo DIALECT "E116"& echo LOAD "tests/test_e116.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- TRS1 ---
set /a TOTAL+=1
echo [%TOTAL%] Testing TRS1 (TRS-80 Level I)...
(echo DIALECT "TRS1"& echo LOAD "tests/test_trs1.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- TRS2 ---
set /a TOTAL+=1
echo [%TOTAL%] Testing TRS2 (TRS-80 Level II)...
(echo DIALECT "TRS2"& echo LOAD "tests/test_trs2.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- C64B ---
set /a TOTAL+=1
echo [%TOTAL%] Testing C64B (Commodore BASIC v2)...
(echo DIALECT "C64B"& echo LOAD "tests/test_c64b.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- ASFT ---
set /a TOTAL+=1
echo [%TOTAL%] Testing ASFT (AppleSoft BASIC)...
(echo DIALECT "ASFT"& echo LOAD "tests/test_asft.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- AINT ---
set /a TOTAL+=1
echo [%TOTAL%] Testing AINT (Apple II Integer BASIC)...
(echo DIALECT "AINT"& echo LOAD "tests/test_aint.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- ATRI ---
set /a TOTAL+=1
echo [%TOTAL%] Testing ATRI (Atari BASIC)...
(echo DIALECT "ATRI"& echo LOAD "tests/test_atri.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

REM --- COCO ---
set /a TOTAL+=1
echo [%TOTAL%] Testing COCO (Color Computer BASIC)...
(echo DIALECT "COCO"& echo LOAD "tests/test_coco.bas"& echo RUN)|..\basicpp.exe 2>&1 | findstr /C:"TESTS COMPLETE" >nul
if %errorlevel%==0 (
    echo     PASS
    set /a PASS+=1
) else (
    echo     FAIL
    set /a FAIL+=1
)

echo.
echo ========================================
echo  Results: %PASS%/%TOTAL% passed, %FAIL% failed
echo ========================================
if %FAIL%==0 (
    echo  ALL DIALECT TESTS PASSED
) else (
    echo  SOME TESTS FAILED
)
echo.
