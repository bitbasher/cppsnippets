# Development Guide & Build Configuration

## Critical Build & Environment Settings

**This file documents essential configuration decisions and workarounds. Read this before building or asking an AI agent for help.**

---

## 1. Parallel Build Limits (CMakeLists.txt, Lines 3-6)

```cmake
# Job pool configuration to prevent system crashes
# Limit compilation to -j4 and linking to -j2 to reduce memory pressure
set_property(GLOBAL APPEND PROPERTY JOB_POOLS compile_pool=4 link_pool=2)
set(CMAKE_JOB_POOL_COMPILE compile_pool)
set(CMAKE_JOB_POOL_LINK link_pool)
```

**Important:** 
- **DO NOT** use `cmake --build . --parallel 8` or higher
- The CMakeLists.txt already restricts builds to 4 compile jobs and 2 link jobs
- This prevents memory exhaustion and system crashes during full builds
- Use: `cmake --build . ` (no parallel flag—respects pool defaults)
- Or: `cmake --build . --parallel 4` (OK, matches pool limit)

**For Agents:** When instructing AI agents to build, use the form without explicit parallel flag or with `--parallel` ≤ 4.

---

## 2. Architecture: Windows MSVC + Visual Studio Generator

**Chosen:** Visual Studio 17 2022 (MSVC) on Windows
**Why:** Best practice for professional Windows applications

**NOT Clangd or Unix-style generators**—though cross-platform appeals, MSVC is the native Windows choice for:
- Native MSBuild integration
- MSVC-specific optimizations and warnings
- Professional Windows toolchain compliance

### Build Command
```powershell
cmake --build "d:\repositories\cppsnippets\cppsnippets\build"
```

---

## 3. VS Code IntelliSense Configuration

**File:** `.vscode/c_cpp_properties.json`

**Key Setting:**
```json
"configurationProvider": "ms-vscode.cmake-tools"
```

**Why this matters:**
- Uses the official Microsoft extension `ms-vscode.cmake-tools`
- Reads Visual Studio project files natively (understands MSVC output)
- Integrates with CMake to find all include paths automatically
- Qt headers, project headers, and system headers discovered correctly

**Previous Setup:** Used `go2sh.cmake-integration` (third-party)—unreliable, required manual include path management.

**Include Paths in c_cpp_properties.json:**
- `${workspaceFolder}/**` - Entire workspace
- `${workspaceFolder}/src/**` - Source files

With `ms-vscode.cmake-tools`, these are sufficient; no need to manually list every library header.

---

## 4. PowerShell Environment Notes

**This is a Windows PowerShell environment, NOT Bash/Linux.**

### Commands That DON'T Work in PowerShell
- `head -20` — NOT available (Unix utility)
- `tail -50` — NOT available (Unix utility)
- `ls -la` — `ls` is aliased to `Get-ChildItem`, but flags differ
- Unix pipes and filters often behave unpredictably

### PowerShell Equivalents
| Unix | PowerShell |
|------|-----------|
| `head -n 20` | `Select-Object -First 20` |
| `tail -n 50` | `Select-Object -Last 50` |
| `ls -la file` | `Get-Item file` or `Test-Path file` |
| `grep pattern` | `Select-String -Pattern "pattern"` |
| `\| wc -l` | `\| Measure-Object` |

### For Agents
**When an agent needs to run commands in this environment:**
1. Use native PowerShell cmdlets (`Get-ChildItem`, `Select-Object`, etc.)
2. Avoid Unix utilities (head, tail, grep, wc, etc.)
3. Use `Select-Object -Last N` for showing output tails
4. Use `Select-Object -First N` for showing output heads

**Agent Instruction:** "This is a PowerShell environment. Avoid Unix commands; use native PowerShell cmdlets instead."

---

## 5. Qt Header Discovery

**Problem Solved:** Qt headers (QWidget, QFile, etc.) marked as "undefined" in VS Code
**Solution:** Automatic via `ms-vscode.cmake-tools` + CMake's include path discovery

- The CMakeLists.txt sets `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)`
- The CMake Tools extension reads the project and includes Qt6 paths automatically
- VS Code IntelliSense now sees Qt6 headers without manual configuration

**If Qt headers still show red squiggles:**
1. Rebuild the project: `cmake --build .`
2. Reload VS Code window (Ctrl+K Ctrl+R or F1 → "Developer: Reload Window")
3. Check that `cmake-tools` extension is installed and active

---

## 6. Resource Paths Configuration (resourcePaths.cpp)

### Suffix Indicator Pattern
Paths ending with `/` → app name + suffix appended  
Paths without `/` → scanned directly without suffix

**Example:**
- `../share/` → becomes `../share/openscad` (or `../share/openscad (Nightly)` if suffix = " (Nightly)")
- `../..` → used as-is, no suffix appended

### User-Level Search Paths (Platform-Specific)

**Windows:**
```cpp
QStringLiteral("."),        // OpenSCAD config folder
QStringLiteral("../")       // Documents base (CSIDL_PERSONAL) with suffix
```

**macOS:**
```cpp
QStringLiteral("."),                  // OpenSCAD config folder
QStringLiteral("../../Documents/")    // NSDocumentDirectory with suffix
```

**Linux:**
```cpp
QStringLiteral("."),                    // OpenSCAD config folder
QStringLiteral("../../.local/share/")   // $HOME/.local/share with suffix
```

---

## 7. Template Tree Model Columns

**Three columns in TemplateTreeModel:**
| Column | Header | Content |
|--------|--------|---------|
| 0 | "Name" | Display name (tier/location/template) |
| 1 | "Category" | Template category |
| 2 | "Name" | Template name from JSON object |

**Not "Path"!** Column 2 shows the template name from the JSON, not a filesystem path.

---

## 8. Test Suite

**All Tests:** 85/85 passing

**Run Tests:**
```powershell
cd build
ctest -C Debug --output-on-failure
```

**Key Test Categories:**
- Resource discovery (all platforms)
- Template tree model (UI)
- Resource scanner (directory listing)
- Resource store (in-memory inventory)

---

## 9. Git Workflow

**Recent Commits:**
1. "Refactor resource path resolution with suffix indicator pattern..." — Path handling redesign
2. "Correct column header..." — UI model fix

**When committing changes:**
- Use descriptive commit messages
- Reference doc changes or test updates in the message
- All tests should pass before committing

---

## Session Notes for AI Agents

**When working with this project, an agent should:**

1. **Use correct build command:** `cmake --build . ` (respects pool limits)
2. **Avoid Unix commands in PowerShell:** Use `Select-Object -Last N`, not `tail -N`
3. **Acknowledge MSVC/Visual Studio:** Don't suggest Clang, Ninja, or Unix generators
4. **Check c_cpp_properties.json:** Verify `ms-vscode.cmake-tools` is the provider
5. **Run full test suite after changes:** `ctest -C Debug --output-on-failure`
6. **Commit with clear messages** referencing what was changed and why
7. **Update this DEVELOPMENT.md** if new build settings or workarounds are added

---

## Quick Reference Checklist

- [ ] CMakeLists.txt job pools limit to -j4 compile, -j2 link (lines 3-6)
- [ ] Using Visual Studio 17 2022 generator (MSVC)
- [ ] c_cpp_properties.json uses `ms-vscode.cmake-tools`
- [ ] PowerShell: use `Select-Object` not `head`/`tail`
- [ ] All tests passing (85/85)
- [ ] Documentation updated in DEVELOPMENT.md

---

**Last Updated:** December 21, 2025
**Project:** ScadTemplates
**Status:** Active Development
