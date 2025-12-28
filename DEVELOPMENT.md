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


---
applyTo: "pwsh"
---
## 4. PowerShell Environment Notes

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

## 5.5 Qt C++ Required Coding Practice

now i am going to paste some example code here so you can see how we need to change the existing GUI app .. this is code to make an 

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
---


---

## 8. Test Suite

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

3. **Acknowledge MSVC/Visual Studio:** Don't suggest Clang, or Unix generators
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
