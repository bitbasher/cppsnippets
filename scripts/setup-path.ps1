#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Setup project scripts in PATH
.DESCRIPTION
    Adds the scripts folder to PATH for the current PowerShell session.
    Optionally adds to PowerShell profile for persistence.
.PARAMETER Persistent
    Add to PowerShell profile so scripts are available in all future sessions
.EXAMPLE
    .\scripts\setup-path.ps1
    
    Adds to PATH for current session only
.EXAMPLE
    .\scripts\setup-path.ps1 -Persistent
    
    Adds to PowerShell profile ($PROFILE) for all future sessions
#>

param(
    [switch]$Persistent
)

$ErrorActionPreference = "Stop"

$ScriptsDir = $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptsDir

Write-Host "Project Scripts Setup" -ForegroundColor Cyan
Write-Host "=====================" -ForegroundColor Cyan
Write-Host ""

# Add to current session
if ($env:PATH -notlike "*$ScriptsDir*") {
    $env:PATH = "$ScriptsDir;$env:PATH"
    Write-Host "✅ Added scripts to PATH for current session" -ForegroundColor Green
} else {
    Write-Host "ℹ️  Scripts already in PATH for current session" -ForegroundColor Yellow
}

# Add to profile if requested
if ($Persistent) {
    Write-Host ""
    Write-Host "Adding to PowerShell profile: $PROFILE" -ForegroundColor Cyan
    
    # Create profile if it doesn't exist
    if (-not (Test-Path $PROFILE)) {
        New-Item -Path $PROFILE -ItemType File -Force | Out-Null
        Write-Host "Created new profile file" -ForegroundColor Gray
    }
    
    # Check if already in profile
    $profileContent = Get-Content $PROFILE -Raw -ErrorAction SilentlyContinue
    if ($profileContent -like "*$ScriptsDir*") {
        Write-Host "ℹ️  Scripts path already in profile" -ForegroundColor Yellow
    } else {
        # Add to profile
        $pathLine = "`$env:PATH = `"$ScriptsDir;`$env:PATH`""
        Add-Content -Path $PROFILE -Value "`n# cppsnippets project scripts (added $(Get-Date -Format 'yyyy-MM-dd'))"
        Add-Content -Path $PROFILE -Value $pathLine
        Write-Host "✅ Added to PowerShell profile" -ForegroundColor Green
        Write-Host "   Restart PowerShell or run: . `$PROFILE" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "Available commands:" -ForegroundColor Cyan
Write-Host "  run-all-unit-tests      - Run all 92 unit tests" -ForegroundColor White
Write-Host "  build-all-unit-tests    - Build Google Test suite" -ForegroundColor White
Write-Host "  build-all-tests         - Build all tests and utilities" -ForegroundColor White
Write-Host ""
Write-Host "Test it now:" -ForegroundColor Cyan
Write-Host "  run-all-unit-tests" -ForegroundColor Gray
