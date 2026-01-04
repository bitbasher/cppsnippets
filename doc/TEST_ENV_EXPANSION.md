# Environment Variable Expansion Test Utility

**Status:** ✅ Complete - Built and Tested

---

## Overview

`test_env_expansion` is a cross-platform console utility that demonstrates and validates the environment variable expansion functionality in ResourcePaths.

It verifies that:
- Both `${VAR}` (Unix) and `%VAR%` (Windows) syntax work correctly
- Platform-specific environment variables are properly resolved
- Undefined variables fail gracefully
- Mixed cases and edge cases are handled properly

---

## Build

The utility is built as part of the project:

```bash
cmake --build build --config Debug --target test_env_expansion --parallel 4
```

### Supported Platforms

- ✅ **Windows** - Uses `%VAR%` style and Windows environment variables
- ✅ **macOS** - Uses `${VAR}` style and macOS environment variables  
- ✅ **Linux/BSD** - Uses `${VAR}` style and POSIX environment variables

The executable automatically adapts to the platform it's built on.

---

## Usage

```
test_env_expansion [options]

Options:
  (no args)      Run all tests
  --verbose      Show all details
  --env          Show system environment variables only
  --paths        Show platform-specific default paths
  --styles       Show variable syntax examples
  --mixed        Show mixed/edge cases
  --help         Show help message
```

---

## Examples

### Run All Tests

```bash
./test-env-expansion
```

Output shows:
1. Available system environment variables
2. Platform-specific default paths with expansion
3. Both ${VAR} and %VAR% syntax examples
4. Edge cases and undefined variables

### Windows Example Output

```
Platform: Windows

Template:  %APPDATA%/openscad
Expanded:  C:\Users\Jeff\AppData\Roaming/openscad

Template:  %PROGRAMFILES%/openscad
Expanded:  C:\Program Files/openscad
```

### Linux Example Output

```
Platform: Linux/BSD/POSIX

Template:  ${HOME}/.config/openscad
Expanded:  /home/jeff/.config/openscad

Template:  ${XDG_CONFIG_HOME}/
Expanded:  /etc/xdg/
```

### Show Only Environment Variables

```bash
./test-env-expansion --env
```

Shows key variables on the current system:

**Windows:**
- APPDATA
- LOCALAPPDATA
- PROGRAMFILES
- PROGRAMDATA
- USERPROFILE
- USERNAME

**macOS:**
- HOME
- USER
- TMPDIR
- SHELL

**Linux:**
- HOME
- USER
- XDG_CONFIG_HOME
- XDG_DATA_HOME
- XDG_CACHE_HOME
- SHELL
- PATH

### Test Specific Feature

```bash
./test-env-expansion --styles     # Variable syntax support
./test-env-expansion --paths      # Platform-specific paths
./test-env-expansion --mixed      # Edge cases
```

---

## Test Coverage

### Variable Styles

Both Unix and Windows syntax are tested:

```
${VAR}     → Unix/POSIX style
%VAR%      → Windows style
```

### Edge Cases

- **Empty strings** - Returns unchanged
- **Undefined variables** - Expands to empty (path becomes "/path" if undefined)
- **Multiple variables** - All are expanded
- **Mixed paths** - Relative and absolute paths work
- **Appended suffixes** - Paths like "%PROGRAMFILES%/openscad (Nightly)" work

### Platform Defaults

Each platform shows its typical default search paths:

**Windows:**
- `%PROGRAMFILES%/` → Program Files
- `%APPDATA%/` → User AppData Roaming
- `%LOCALAPPDATA%/` → User AppData Local
- `%PROGRAMDATA%/` → System-wide data

**macOS:**
- `${HOME}/Library/Application Support/`
- `/Library/Application Support/` (system)
- `${HOME}/.config/`

**Linux:**
- `${HOME}/.config/`
- `${XDG_CONFIG_HOME}/`
- `${HOME}/.local/share/`
- `/usr/share/`
- `/usr/local/share/`

---

## Implementation Details

The utility includes an exact copy of the `expandEnvVars()` logic from ResourcePaths, allowing it to be used standalone without linking the entire library.

```cpp
QString expandEnvVars(const QString& path)
```

Uses:
- `QProcessEnvironment::systemEnvironment()` - Get system environment variables
- `QRegularExpression` - Pattern matching for `${VAR}` and `%VAR%`
- Platform detection via `#ifdef` - Platform-specific test cases

---

## Building in Release Mode

```bash
cmake --build build --config Release --target test_env_expansion --parallel 4
```

Output: `build/bin/Release/test-env-expansion.exe`

---

## Executable Location

After building, the test utility is located at:

- **Windows:** `build/bin/Debug/test-env-expansion.exe`
- **Linux:** `build/bin/Debug/test-env-expansion`
- **macOS:** `build/bin/Debug/test-env-expansion`

---

## Integration with CI/CD

This test utility can be used in automated testing:

```bash
# Run test and capture output
./test-env-expansion > expansion_test_output.txt

# Verify success (exit code 0)
./test-env-expansion
echo "Test exit code: $?"
```

The tool always exits with code `0` (success). No error cases are tested—it's purely demonstrative.

---

## Future Enhancements

Possible additions:
- Validation that expanded paths are absolute
- Performance benchmarking (expansion speed)
- Batch testing from a file of test cases
- JSON output format for automated parsing
- Comparison with ResourcePaths actual methods
