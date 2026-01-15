# Project Scripts

This folder contains PowerShell scripts for common development tasks.

## Setup

Add this folder to your PowerShell session PATH:

```powershell
# Temporary (current session only)
$env:PATH = "$(Get-Location)\scripts;$env:PATH"

# Or add to your PowerShell profile for persistence
Add-Content $PROFILE "`n# cppsnippets project scripts`n`$env:PATH = `"D:\repositories\cppsnippets\cppsnippets\scripts;`$env:PATH`""
```

## Available Scripts

### Test Scripts

- **`run-all-unit-tests.ps1`** - Run all unit tests via ctest (92 Google Test cases)
- **`build-all-unit-tests.ps1`** - Build the Google Test suite
- **`build-all-tests.ps1`** - Build all tests and standalone utilities

### Usage Examples

```powershell
# After adding scripts to PATH:
run-all-unit-tests
build-all-unit-tests
build-all-tests

# Or run directly:
.\scripts\run-all-unit-tests.ps1
.\scripts\build-all-unit-tests.ps1
```

## Requirements

- PowerShell 7+ (pwsh)
- CMake configured with `cmake --preset vs2022`
- Build directory at `build/`
