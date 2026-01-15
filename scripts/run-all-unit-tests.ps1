#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Run all unit tests via ctest
.DESCRIPTION
    Runs the Google Test suite (92 tests) from the build directory
.EXAMPLE
    run-all-unit-tests
#>

$ErrorActionPreference = "Stop"

# Get project root (parent of scripts directory)
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

if (-not (Test-Path $BuildDir)) {
    Write-Error "Build directory not found at: $BuildDir"
    Write-Error "Run cmake --preset vs2022 first"
    exit 1
}

# Change to build directory and run tests
Push-Location $BuildDir
try {
    Write-Host "Running all unit tests from: $BuildDir" -ForegroundColor Cyan
    & ctest -C Debug --output-on-failure
    $exitCode = $LASTEXITCODE
    
    if ($exitCode -eq 0) {
        Write-Host "`n✅ All tests passed!" -ForegroundColor Green
    } else {
        Write-Host "`n❌ Some tests failed (exit code: $exitCode)" -ForegroundColor Red
    }
    
    exit $exitCode
} finally {
    Pop-Location
}
