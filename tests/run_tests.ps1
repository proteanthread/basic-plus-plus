# BASIC++ Comprehensive Test Suite Runner
# Runs all .BAS and .BPP test files through basicpp.exe
# and checks for FAIL output.

$ErrorActionPreference = "Continue"
$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$project_root = Split-Path -Parent $script_dir
$exe = Join-Path $project_root "basicpp.exe"

if (-not (Test-Path $exe)) {
    Write-Host "ERROR: basicpp.exe not found at $exe" -ForegroundColor Red
    exit 1
}

$test_files = @(Get-ChildItem -Path $script_dir -Filter "*.bas" -File) + @(Get-ChildItem -Path $script_dir -Filter "*.bpp" -File) | Sort-Object Name
$total = 0
$passed = 0
$failed = 0
$errors = @()

Write-Host ""
Write-Host "===== BASIC++ TEST SUITE =====" -ForegroundColor Cyan
Write-Host "Executable: $exe"
Write-Host "Tests: $($test_files.Count) files"
Write-Host ""

foreach ($tf in $test_files) {
    $total++
    $name = $tf.Name
    
    # Build commands to pipe: LOAD the file, RUN it, then BYE
    $commands = "LOAD `"$($tf.FullName -replace '\\','/')`"`nRUN`nBYE`n"
    
    # Run with timeout
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo.FileName = $exe
    $proc.StartInfo.UseShellExecute = $false
    $proc.StartInfo.RedirectStandardInput = $true
    $proc.StartInfo.RedirectStandardOutput = $true
    $proc.StartInfo.RedirectStandardError = $true
    $proc.StartInfo.CreateNoWindow = $true
    
    try {
        $proc.Start() | Out-Null
        $proc.StandardInput.Write($commands)
        $proc.StandardInput.Close()
        
        $output = $proc.StandardOutput.ReadToEnd()
        $stderr_out = $proc.StandardError.ReadToEnd()
        $proc.WaitForExit(15000)  # 15 second timeout
        
        if (-not $proc.HasExited) {
            $proc.Kill()
            Write-Host "  TIMEOUT  $name" -ForegroundColor Yellow
            $failed++
            $errors += "$name (TIMEOUT)"
            continue
        }
        
        # Check for FAIL lines in output
        $fail_lines = $output -split "`n" | Where-Object { $_ -match "^FAIL" }
        $err_lines = $output -split "`n" | Where-Object { $_ -match "^(WHAT|HOW|SORRY)" }
        
        if ($fail_lines.Count -gt 0) {
            Write-Host "  FAIL     $name" -ForegroundColor Red
            foreach ($fl in $fail_lines) {
                Write-Host "           $($fl.Trim())" -ForegroundColor DarkRed
            }
            $failed++
            $errors += $name
        } elseif ($err_lines.Count -gt 0 -and $name -notmatch "error_handling") {
            # t17 tests error handling, so errors are expected
            Write-Host "  ERROR    $name" -ForegroundColor Yellow
            foreach ($el in $err_lines) {
                Write-Host "           $($el.Trim())" -ForegroundColor DarkYellow
            }
            $failed++
            $errors += "$name (ERROR)"
        } else {
            Write-Host "  PASS     $name" -ForegroundColor Green
            $passed++
        }
    }
    catch {
        Write-Host "  CRASH    $name" -ForegroundColor Red
        Write-Host "           $($_.Exception.Message)" -ForegroundColor DarkRed
        $failed++
        $errors += "$name (CRASH)"
    }
    finally {
        if ($proc -and -not $proc.HasExited) {
            try { $proc.Kill() } catch {}
        }
        $proc.Dispose()
    }
}

Write-Host ""
Write-Host "===== RESULTS =====" -ForegroundColor Cyan
Write-Host "Total:  $total"
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })

if ($errors.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed tests:" -ForegroundColor Red
    foreach ($e in $errors) {
        Write-Host "  - $e" -ForegroundColor Red
    }
}

Write-Host ""
exit $failed
