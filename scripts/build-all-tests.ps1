#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Build all tests (unit tests + standalone utilities)
.DESCRIPTION
    Builds the Google Test suite and standalone test utilities
    (test_env_expansion, test_path_discovery, test_tier_standalone, settings_generator)
.EXAMPLE
    build-all-tests
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

Write-Host "Building all tests and utilities..." -ForegroundColor Cyan
& cmake --build $BuildDir --target build-all-tests --config Debug --parallel 4
$exitCode = $LASTEXITCODE

if ($exitCode -eq 0) {
    Write-Host "`n✅ Build successful!" -ForegroundColor Green
} else {
    Write-Host "`n❌ Build failed (exit code: $exitCode)" -ForegroundColor Red
}

exit $exitCode
