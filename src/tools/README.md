# Tools Directory

This directory contains utility applications for testing and configuring the ScadTemplates application.

---

## settings-generator

Command-line utility for managing user-designated resource search paths.

### Purpose

Allows you to configure additional locations where ScadTemplates should search for resources. These paths are stored in QSettings (Windows Registry or platform-specific config files) and are automatically loaded by the main application at startup.

### Usage

```powershell
settings-generator [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--add <path>` or `-a` | Add a user-designated path |
| `--clear` or `-c` | Clear all user-designated paths |
| `--list` or `-l` | List current user-designated paths |
| `--default` or `-d` | Add platform-specific default test paths |
| `--help` or `-h` | Show help message |

### Examples

**Add a custom resource path:**
```powershell
settings-generator --add "C:/CustomOpenSCAD"
settings-generator --add "%USERPROFILE%/MyTemplates"
settings-generator --add "D:/Projects/ScadLibs"
```

**List configured paths:**
```powershell
settings-generator --list
```

Output:
```
=== Current User-Designated Paths ===

  [1] C:/CustomOpenSCAD
  [2] %USERPROFILE%/MyTemplates
  [3] D:/Projects/ScadLibs

Total: 3 path(s)

Settings stored at: \HKEY_CURRENT_USER\Software\ScadTemplates\ResourcePaths
```

**Add default test paths:**
```powershell
settings-generator --default
```

This adds platform-specific test paths:
- **Windows**: `C:/CustomScad`, `%USERPROFILE%/Documents/MyTemplates`, `D:/ProjectResources/ScadLibs`
- **macOS**: `/Applications/CustomSCAD`, `${HOME}/Documents/MyTemplates`, `/Volumes/External/ScadLibs`
- **Linux**: `/opt/customscad`, `${HOME}/scad-templates`, `/usr/local/custom-scad`

**Clear all paths:**
```powershell
settings-generator --clear
```

### Environment Variable Support

Paths can include environment variables that will be expanded at runtime:

- **Windows**: `%USERPROFILE%`, `%APPDATA%`, `%LOCALAPPDATA%`, `%PROGRAMFILES%`
- **Unix/macOS**: `${HOME}`, `${XDG_CONFIG_HOME}`

Example:
```powershell
settings-generator --add "%USERPROFILE%/Documents/OpenSCAD"
```

### Storage Location

Settings are stored in platform-specific locations:

- **Windows**: `HKEY_CURRENT_USER\Software\ScadTemplates\ResourcePaths`
- **macOS**: `~/Library/Preferences/com.ScadTemplates.ResourcePaths.plist`
- **Linux**: `~/.config/ScadTemplates/ResourcePaths.conf`

### Integration with Main Application

User-designated paths are automatically loaded by `ResourcePaths::userDesignatedPaths()` and included in the Installation tier search paths when `ResourcePaths::qualifiedSearchPaths()` is called.

The main application searches for resources in this order:
1. Default installation paths (with suffix, e.g., "ScadTemplates (Nightly)")
2. Sibling installations (LTS ↔ Nightly automatic discovery)
3. **User-designated paths** (from settings)
4. Machine tier paths (system-wide)
5. User tier paths (user-specific)

### Building

The utility is built as an excluded target (not part of default build):

```powershell
cmake --build . --config Debug --target settings_generator --parallel 4
```

Executable: `build/bin/Debug/settings-generator.exe` (Windows)

---

## test-env-expansion

Console utility for testing environment variable expansion and path qualification.

### Purpose

Validates the environment variable expansion logic, folder name appending rules, and path canonicalization used by `ResourcePaths`.

### Usage

```powershell
test-env-expansion [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--env` | Test environment variable expansion |
| `--paths` | Test platform-specific default paths |
| `--qualified` | Test qualified paths (env vars + folder names) |
| `--styles` | Test variable style syntax (${VAR} vs %VAR%) |
| `--mixed` | Test edge cases and mixed scenarios |
| `--help` | Show help message |

Run without options to execute all tests.

### Examples

**Test qualified paths:**
```powershell
test-env-expansion --qualified
```

Output:
```
======================================================================
Qualified Paths (Env Vars + Folder Names)
======================================================================

Platform: Windows

=== Installation Tier (with suffix) ===
Template:  %PROGRAMFILES%/
Qualified: C:/Program Files/ScadTemplates (Nightly)

Template:  .
Qualified: D:/repositories/cppsnippets/cppsnippets/build/bin/Debug

Template:  ../share/
Qualified: D:/repositories/cppsnippets/cppsnippets/build/bin/share/ScadTemplates (Nightly)
```

**Test edge cases:**
```powershell
test-env-expansion --mixed
```

Validates:
- Undefined variables (expand to empty string)
- Relative paths with `..` components (resolve to absolute)
- Mixed path separators (normalized to forward slashes)
- Empty paths and trailing slashes

### Building

```powershell
cmake --build . --config Debug --target test_env_expansion --parallel 4
```

Executable: `build/bin/Debug/test-env-expansion.exe` (Windows)

---

## Development Notes

### Adding New Utilities

1. Create source file in `src/tools/`
2. Add CMake target in root `CMakeLists.txt`:
   ```cmake
   add_executable(my_tool EXCLUDE_FROM_ALL src/tools/my_tool.cpp)
   target_link_libraries(my_tool PRIVATE Qt6::Core)
   set_target_properties(my_tool PROPERTIES
       OUTPUT_NAME my-tool
       WIN32_EXECUTABLE OFF
   )
   ```
3. Document in this README

### Testing Utilities

All tools should be excluded from the default build (`EXCLUDE_FROM_ALL`) and built explicitly when needed:

```powershell
cmake --build . --config Debug --target <tool_name> --parallel 4
```
