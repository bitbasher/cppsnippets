#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Build all unit tests
.DESCRIPTION
    Builds the Google Test suite (scadtemplates_tests target)
.EXAMPLE
    build-all-unit-tests
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

Write-Host "Building all unit tests..." -ForegroundColor Cyan
& cmake --build $BuildDir --target build-all-unit-tests --config Debug --parallel 4
$exitCode = $LASTEXITCODE

if ($exitCode -eq 0) {
    Write-Host "`n✅ Build successful!" -ForegroundColor Green
} else {
    Write-Host "`n❌ Build failed (exit code: $exitCode)" -ForegroundColor Red
}

exit $exitCode
