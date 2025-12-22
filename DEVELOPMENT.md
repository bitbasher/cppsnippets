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
5. The ^ line continuation does not work in powershell - no multi-line commands

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

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(const QFileInfo &fileInfo, QWidget *parent = nullptr);
};
```
### GeneralTab Class Implementation

The GeneralTab widget simply displays some information about the file passed by the TabDialog. Various widgets for this purpose, and these are arranged within a vertical layout:
```cpp
GeneralTab::GeneralTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QLabel *fileNameLabel = new QLabel(tr("File Name:"));
    QLineEdit *fileNameEdit = new QLineEdit(fileInfo.fileName());

    QLabel *pathLabel = new QLabel(tr("Path:"));
    QLabel *pathValueLabel = new QLabel(fileInfo.absoluteFilePath());
    pathValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *sizeLabel = new QLabel(tr("Size:"));
    qlonglong size = fileInfo.size()/1024;
    QLabel *sizeValueLabel = new QLabel(tr("%1 K").arg(size));
    sizeValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *lastReadLabel = new QLabel(tr("Last Read:"));
    QLabel *lastReadValueLabel = new QLabel(fileInfo.lastRead().toString());
    lastReadValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *lastModLabel = new QLabel(tr("Last Modified:"));
    QLabel *lastModValueLabel = new QLabel(fileInfo.lastModified().toString());
    lastModValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(fileNameEdit);
    mainLayout->addWidget(pathLabel);
    mainLayout->addWidget(pathValueLabel);
    mainLayout->addWidget(sizeLabel);
    mainLayout->addWidget(sizeValueLabel);
    mainLayout->addWidget(lastReadLabel);
    mainLayout->addWidget(lastReadValueLabel);
    mainLayout->addWidget(lastModLabel);
    mainLayout->addWidget(lastModValueLabel);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
```
### PermissionsTab Class Definition

Like the GeneralTab, the PermissionsTab is just used as a placeholder widget for its children:
```cpp
class PermissionsTab : public QWidget
{
    Q_OBJECT

public:
    explicit PermissionsTab(const QFileInfo &fileInfo, QWidget *parent = nullptr);
};
```
### PermissionsTab Class Implementation

The PermissionsTab shows information about the file's access information, displaying details of the file permissions and owner in widgets that are arranged in nested layouts:
```cpp
PermissionsTab::PermissionsTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *permissionsGroup = new QGroupBox(tr("Permissions"));

    QCheckBox *readable = new QCheckBox(tr("Readable"));
    if (fileInfo.isReadable())
        readable->setChecked(true);

    QCheckBox *writable = new QCheckBox(tr("Writable"));
    if ( fileInfo.isWritable() )
        writable->setChecked(true);

    QCheckBox *executable = new QCheckBox(tr("Executable"));
    if ( fileInfo.isExecutable() )
        executable->setChecked(true);

    QGroupBox *ownerGroup = new QGroupBox(tr("Ownership"));

    QLabel *ownerLabel = new QLabel(tr("Owner"));
    QLabel *ownerValueLabel = new QLabel(fileInfo.owner());
    ownerValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *groupLabel = new QLabel(tr("Group"));
    QLabel *groupValueLabel = new QLabel(fileInfo.group());
    groupValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QVBoxLayout *permissionsLayout = new QVBoxLayout;
    permissionsLayout->addWidget(readable);
    permissionsLayout->addWidget(writable);
    permissionsLayout->addWidget(executable);
    permissionsGroup->setLayout(permissionsLayout);

    QVBoxLayout *ownerLayout = new QVBoxLayout;
    ownerLayout->addWidget(ownerLabel);
    ownerLayout->addWidget(ownerValueLabel);
    ownerLayout->addWidget(groupLabel);
    ownerLayout->addWidget(groupValueLabel);
    ownerGroup->setLayout(ownerLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(permissionsGroup);
    mainLayout->addWidget(ownerGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
```
### ApplicationsTab

The ApplicationsTab is another placeholder widget that is mostly cosmetic:
```cpp
class ApplicationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ApplicationsTab(const QFileInfo &fileInfo, QWidget *parent = nullptr);
};
```
### ApplicationsTab Class Implementation

The ApplicationsTab does not show any useful information, but could be used as a template for a more complicated example:
```cpp
ApplicationsTab::ApplicationsTab(const QFileInfo &fileInfo, QWidget *parent)
    : QWidget(parent)
{
    QLabel *topLabel = new QLabel(tr("Open with:"));

    QListWidget *applicationsListBox = new QListWidget;
    QStringList applications;

    for (int i = 1; i <= 30; ++i)
        applications.append(tr("Application %1").arg(i));
    applicationsListBox->insertItems(0, applications);

    QCheckBox *alwaysCheckBox;

    if (fileInfo.suffix().isEmpty())
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
            "open this type of file"));
    else
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
            "open files with the extension '%1'").arg(fileInfo.suffix()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(applicationsListBox);
    layout->addWidget(alwaysCheckBox);
    setLayout(layout);
}
```
you see how the layouts are applied last ?

that is how our app needs to it

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
| 0 | "Tier" | Display name |
| 1 | "Location" | resource location |
| 2 | "Name" | Template name from JSON object |

**Not "Path"!** Column 2 shows the name of the JSON object, not the content of prefix.

---

## 8. Test Suite

there is an ongoing issue with test discovery reporting problems .. the ctest section of the cmakelists is looking for tests that have not yet been built

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
