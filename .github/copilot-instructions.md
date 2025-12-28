# Development Guide & Build Configuration

---
applyTo: "**"
---
# Project general coding standards

## Naming Conventions
- Use PascalCase for File names and Class names
- Use camelCase for variables, functions, and methods
- Prefix private class members with "m_"

## Error Handling
- Use try/catch blocks for async operations
- Always log errors with contextual information
---

---
applyTo: "cpp, cc, cxx, c++, h, hh, hpp, h++"
---

## C++ guidelines

- prefer `nullptr` over `NULL` or `0` for pointers
- use `override` keyword for overridden virtual functions
- prefer .cpp extension for source files
- prefer .hpp extension for header files
- prefer inline member functions in header files for small functions
- always provide a no argument constructor to a class
- always provide a copy constructor to a class
- always provide an operator = method to a class
- use `explicit` keyword for single-argument constructors
- use `final` keyword for classes that should not be inherited from
- an enum class should have an Unknown item of value 0
- prefer `QList` over `QVector` for dynamic arrays
- replace `QVector` with `QList` when refactoring
- prefer `QStringLiteral` for string constants
- use `const` and `&` for function parameters where appropriate
- use `auto` for type inference where appropriate

---
applyTo: "CMakeLists.txt"
---
## 1. Parallel Build Limits (CMakeLists.txt, Lines 3-6)
- Add the following code block after the first comment block in the file
```cmake
# Job pool configuration to prevent system crashes
# Limit compilation to -j4 and linking to -j2 to reduce memory pressure
set_property(GLOBAL APPEND PROPERTY JOB_POOLS compile_pool=4 link_pool=2)
set(CMAKE_JOB_POOL_COMPILE compile_pool)
set(CMAKE_JOB_POOL_LINK link_pool)
```

**Why:** Full parallel builds can exhaust system memory, causing crashes. Limiting to 4 compile jobs and 2 link jobs balances speed and stability.
---

---
applyTo: "cmake"
---
- Use maximum 4 on --parallel option to cmake command
- `cmake --build . ` (limits enforced by defaults set in CMakeLists.txt)
- `cmake --build . --parallel 4` (OK, matches pool limit)

**For Agents:** When instructing AI agents to "cmake --build ..." use EITHER 
  NO --parallel option OR  `--parallel 4` at most

**Why:** parallel builds using all available cores can exhaust system memory, causing crashes. Limiting to 4 compile jobs and 2 link jobs balances speed and stability.
---

## 2. Architecture & IDE: Visual Studio 2022/2026

**Primary IDE:** Visual Studio 2022 or 2026 (Full IDE, NOT VS Code)
**Generator:** Visual Studio 17 2022 (MSVC v143 toolset)
**Why:** Best practice for professional Windows applications

**Important:**
- VS 2026 IDE can use the VS 2022 toolchain (no changes needed)
- Generator name refers to compiler toolset, not IDE version
- Do NOT suggest Clang or Unix-style generators

**Build Command:**
```powershell
cmake --build "d:\repositories\cppsnippets\cppsnippets\build" --config Debug --parallel 4
```

**CMake Configuration:**
Uses `CMakePresets.json` (modern standard, replaces deprecated CMakeSettings.json)

---

## 3. Project Structure & Legacy Code

### src-orig Folder - DO NOT BUILD

**Location:** `D:\repositories\cppsnippets\cppsnippets\src-orig\`
**Purpose:** Reference-only TypeScript source files from VS Code snippet functionality

**CRITICAL:** This folder contains legacy `.ts` files that are **NOT part of the C++ build**

**Files to prevent IDE/build system from processing src-orig:**
1. `.gitignore` - Contains `src-orig/` entry
2. `.vsignore` - Visual Studio-specific exclusion
3. `src-orig/DO_NOT_BUILD.md` - Documentation explaining the folder
4. `.vscode/settings.json` - VS Code exclusion settings

**If IDE tries to build .ts files:**
- Check that `.vsignore` exists at project root
- Verify `.gitignore` has `src-orig/` entry
- In Visual Studio: Solution Explorer → Show All Files → Manually exclude folder

---

## 4. VS Code IntelliSense Configuration (Optional)

**Note:** Primary IDE is Visual Studio, but VS Code can also be used

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

**Include Paths in c_cpp_properties.json:**
- `${workspaceFolder}/**` - Entire workspace
- `${workspaceFolder}/src/**` - Source files

With `ms-vscode.cmake-tools`, these are sufficient; no need to manually list every library header.

**VS Code Extensions (if using VS Code):**
- C/C++ Extension Pack (ms-vscode.cpptools-extension-pack)
- CMake Tools (ms-vscode.cmake-tools)
- CMake Language Support (twxs.cmake)
- C++ TestMate (matepek.vscode-catch2-test-adapter) - for test discovery
- Markdown All in One (yzhang.markdown-all-in-one)
- PowerShell (ms-vscode.powershell)

---

## 5. PowerShell Environment Notes

** Do ONLY commands for PowerShell, unix shell commands are not supported and waste references **
- Use ONLY native PowerShell cmdlets (`Get-ChildItem`, `Select-Object`, etc.)

### Commands That DON'T Work in PowerShell
- `head` — NOT available (Unix utility)
- `tail` — NOT available (Unix utility)
- `ls -la` — NOT available

### Unix pipes and filters do not work and waste references
- replace `head -n 20` with `Select-Object -First 20`
- replace `tail -n 50` with `Select-Object -Last 50`
- replace `ls -la file` with `Get-Item file`
- replace `ls -la file` with `Test-Path file` 
- replace `grep pattern` with `Select-String -Pattern "pattern"`
- replace `\| wc -l`     with `\| Measure-Object`
- The ^ line continuation does not work in powershell - no multi-line commands

---

## 6. Required Dependencies

### Qt 6.10.1
**Location:** `C:/bin/Qt/6.10.1/msvc2022_64`
**Components Required:**
- Qt6::Core (QString, QVector, QSettings, QCoreApplication)
- Qt6::Gui (QScreen, platform detection)
- Qt6::Widgets (GUI application)
- Qt6::Test (unit testing framework)

**CMake Detection:**
CMakeLists.txt auto-detects Qt based on compiler:
```cmake
set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.1/msvc2022_64")
```

**Manual Override:**
```powershell
cmake -DCMAKE_PREFIX_PATH="C:/path/to/Qt6" ..
```

### vcpkg Dependencies
**Location:** `C:/bin/vcpkg/`
**Installed Packages:**
- nlohmann-json (JSON parsing)
- json-schema-validator (JSON schema validation)

**Toolchain File:**
```cmake
-DCMAKE_TOOLCHAIN_FILE=C:/bin/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Google Test
**Version:** 1.14.0
**Source:** Fetched automatically via CMake FetchContent
**No manual installation required**

### Libraries Built In-Project
1. **JsonReaderPortable** - `src/jsonreader/jsonreader-portable/`
2. **JsonWriterPortable** - `src/jsonwriter/jsonwriter-portable/`

---

## 7. Build Process From Scratch

### Initial Configuration
```powershell
# Navigate to project root
cd D:\repositories\cppsnippets\cppsnippets

# Configure with CMake preset (recommended)
cmake --preset vs2022

# OR configure manually
cmake -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:/bin/Qt/6.10.1/msvc2022_64" `
  -DCMAKE_TOOLCHAIN_FILE="C:/bin/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DBUILD_APP=ON -DBUILD_TESTS=ON -DBUILD_SHARED_LIBS=ON `
  -S . -B build
```

### Build
```powershell
# Build Debug configuration (respects -j4 limit from CMakeLists.txt)
cmake --build build --config Debug --parallel 4

# Build Release configuration
cmake --build build --config Release --parallel 4
```

### Clean Build (if needed)
```powershell
# Remove cache and rebuild
Remove-Item build\CMakeCache.txt -Force
Remove-Item build\CMakeFiles -Recurse -Force
cmake --preset vs2022
cmake --build build --config Debug --parallel 4
```

### Output Locations
- **Libraries:** `build/lib/Debug/` or `build/lib/Release/`
  - `resourceMgmt.dll` - Platform info & resource inventory
  - `scadtemplates.dll` - Template management
- **Executables:** `build/bin/Debug/` or `build/bin/Release/`
  - `scadtemplates.exe` - Main GUI application
- **Qt Plugins:** Automatically copied to `build/bin/Debug/platforms/`

---

## 8. Qt C++ Required Coding Practice

### Example TabDialog

```cpp
TabDialog::TabDialog(const QString &fileName, QWidget *parent)
    : QDialog(parent)
{
    QFileInfo fileInfo(fileName);

    tabWidget = new QTabWidget;
    tabWidget->addTab(new GeneralTab(fileInfo), tr("General"));
    tabWidget->addTab(new PermissionsTab(fileInfo), tr("Permissions"));
    tabWidget->addTab(new ApplicationsTab(fileInfo), tr("Applications"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Tab Dialog"));
}
```

---

## 9. Test Suite

There is an ongoing issue with test discovery reporting problems .. the ctest section of the cmakelists is looking for tests that have not yet been built

The build now succeeds with Ninja. The fix was using `DISCOVERY_MODE PRE_TEST` which defers test discovery until `ctest` runs instead of during the build.

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
- Google Test suite (85+ tests)
- Qt Test suite (GUI and scanner tests)

---

## 10. Documentation & Commit Protocol

### Multi-Phase Refactoring Workflow

**Purpose:** Track progress, capture decisions, minimize loss from reverts

When undertaking multi-step refactorings or significant feature work:

#### Phase 0: Planning Document

**Before any code changes:**

1. **Create Planning Document** in `doc/` folder
   - Filename: `YYYY-MM-DD-feature-or-refactor-name.md`
   - If multiple plans same day: `YYYY-MM-DD-HHmm-feature-name.md`
   
2. **Content Requirements:**
   ```markdown
   # Feature/Refactor Name
   **Date:** YYYY-MM-DD [HH:mm if needed]
   **Author:** [User/Agent]
   **Status:** Planning
   
   ## User Requirements
   [Capture user's specifications verbatim]
   
   ## Problem Statement
   [What needs to change and why]
   
   ## Proposed Solution
   [High-level approach]
   
   ## Multi-Phase Plan
   
   ### Phase 1: [Name]
   **Goal:** [What this achieves]
   **Changes:** [List of modifications]
   **Risk:** [Low/Medium/High]
   
   ### Phase 2: [Name]
   ...
   
   ## Decision Log
   [Capture key architectural decisions with rationale]
   ```

3. **Get User Approval** before proceeding to code changes

4. **Initial Commit:**
   ```powershell
   git add doc/YYYY-MM-DD-feature-name.md
   git commit -m "docs: Add planning doc for [feature name]"
   ```

#### Phase Execution Cycle

For **each phase** of the plan:

1. **Code Changes**
   - Implement phase as planned
   - Update planning doc with any deviations

2. **Create Phase Results Document**
   - Filename: `doc/YYYY-MM-DD-phase[N]-[name]-results.md`
   - Content:
     ```markdown
     # Phase N Complete: [Name]
     **Date:** YYYY-MM-DD
     **Status:** ✅ Complete / ⚠️ Issues / ❌ Failed
     
     ## What Was Done
     [List of changes made]
     
     ## Files Modified
     [List of changed files]
     
     ## Build Status
     ✅/❌ Build result
     
     ## Test Results
     [Pass/fail summary]
     
     ## Issues Encountered
     [Problems discovered and solutions]
     
     ## Architecture Changes
     [Before/after comparison]
     
     ## What's Next
     [Preview of next phase]
     ```

3. **Update Planning Doc**
   - Mark phase as complete: ✅
   - Add "Status" field showing current phase
   - Update decision log with any new discoveries

4. **Commit Documentation First**
   ```powershell
   git add doc/YYYY-MM-DD-*.md
   git commit -m "docs: Complete Phase N - [name]"
   ```

5. **Commit Code Changes**
   ```powershell
   git add [modified files]
   git commit -m "feat/refactor: [Phase N description]

   - [bullet points of changes]
   - [reference planning doc]
   
   Refs: doc/YYYY-MM-DD-feature-name.md"
   ```

6. **Build Verification**
   ```powershell
   cmake --build build --config Debug --parallel 4
   ```

7. **Test Verification**
   ```powershell
   cd build
   ctest -C Debug --output-on-failure
   ```

8. **If Issues Found:**
   - Fix issues
   - **Update results doc** with:
     - Problem description
     - Root cause analysis
     - Solution applied
     - Lessons learned
   - Commit fixes:
     ```powershell
     git add [fixed files] doc/[results-doc]
     git commit -m "fix: [Issue description] in Phase N
     
     - [what was wrong]
     - [how it was fixed]
     
     Updated results doc with findings."
     ```

9. **Known Good Commit**
   ```powershell
   # Tag successful phase completion
   git tag phase-N-complete
   git push origin phase-N-complete
   ```

10. **Proceed to Next Phase** or conclude

#### Why This Protocol Matters

**Prevents Loss:**
- Clear commit points to revert to
- Documentation survives code reverts
- Decision rationale preserved

**Tracks Progress:**
- Each phase is independently verifiable
- Build/test status at each stage
- Easy to resume after interruption

**Captures Knowledge:**
- User specifications preserved
- Architectural decisions documented
- Problems and solutions recorded
- Future maintainers understand "why"

**Examples from This Project:**
- `doc/2025-12-28-resource-location-enablement-refactoring.md` - Planning doc
- `doc/2025-12-29-phase1-unified-discovery.md` - Results doc

### Standard Commit Messages

**Planning:**
```
docs: Add planning doc for [feature name]

Multi-phase plan covering:
- Phase 1: [brief]
- Phase 2: [brief]
- Phase 3: [brief]
```

**Phase Complete:**
```
feat: Complete Phase N - [name]

- [change 1]
- [change 2]
- [change 3]

Build: ✅ Successful
Tests: ✅ Passing

Refs: doc/YYYY-MM-DD-feature-name.md
```

**Bug Fixes During Phase:**
```
fix: [Issue] discovered in Phase N

Problem: [what was wrong]
Cause: [root cause]
Solution: [how fixed]

Updated: doc/YYYY-MM-DD-phaseN-results.md with findings
```

**Known Good Tag:**
```
git tag -a phase-N-complete -m "Phase N: [name] - Build ✅ Tests ✅"
```

---

## 11. Git Workflow

### Daily Commits

**When committing changes:**
- Use descriptive commit messages
- Reference doc changes or test updates in the message
- All tests should pass before committing
- Follow the Documentation & Commit Protocol (Section 10) for multi-phase work

### Commit Message Format

```
<type>: <subject>

<body>

<footer>
```

**Types:**
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation only
- `refactor:` - Code restructuring (no behavior change)
- `test:` - Test additions or modifications
- `build:` - Build system or dependency changes
- `chore:` - Maintenance tasks

**Example:**
```
feat: Add unified resource discovery across all tiers

- Implement discoverAllLocations() as single entry point
- Add tier-specific helpers (Installation/Machine/User)
- Centralize status checking
- Tier tags are now enum markers, not code paths

Build: ✅ Successful
Tests: ⏳ Pending

Refs: doc/2025-12-28-resource-location-enablement-refactoring.md
```

---

## 12. Common Issues & Solutions
