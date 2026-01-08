# QtExploration - QStandardPaths Testing Suite

This folder contains demonstration programs that explore Qt's `QStandardPaths` API to understand how path discovery and placeholder expansion works.

## Programs

1. **locate_demo.cpp** - Demonstrates `QStandardPaths::locate()` for finding single "templates" folders
2. **locateall_demo.cpp** - Demonstrates `QStandardPaths::locateAll()` for finding all "templates" folders  
3. **findexecutable_demo.cpp** - Demonstrates `QStandardPaths::findExecutable()` and `QDirListing` usage
4. **standardlocations_demo.cpp** - Shows all `StandardLocation` types with their paths and existence status
5. **appname_expansion_test.cpp** - Comprehensive test of placeholder tag expansion

## Building

```powershell
cd QtExploration
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

## Running

```powershell
cd build/bin/Debug
./locate_demo.exe
./locateall_demo.exe
./findexecutable_demo.exe
./standardlocations_demo.exe
./appname_expansion_test.exe
```

## Key Findings from Testing

### Placeholder Tag Expansion

Qt uses placeholder tags in path templates that get expanded at runtime:

- **`<USER>`** - Replaced with the Windows username (e.g., "Jeff")
- **`<APPNAME>`** - Actually expands to `<ORGNAME>/<APPNAME>` (two-level directory structure)
- **`<APPDIR>`** - Replaced with the executable directory (portable/fallback paths)

### How to Set Organization and Application Names in C++

```cpp
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set organization name
    app.setOrganizationName("jartisan");
    
    // Set application name
    app.setApplicationName("OpenSCAD");
    
    // Now QStandardPaths will use these values
    // Result: paths like "C:/Users/Jeff/AppData/Local/jartisan/OpenSCAD"
    
    return app.exec();
}
```

### Test Results: Expected vs Actual Tag Expansion

Based on comprehensive testing with organization="jartisan" and application="OpenSCAD":

| Location | Expected Tags (Qt Docs) | Actual Tags Found | Match? |
|----------|------------------------|-------------------|--------|
| DesktopLocation | `<USER>` | `<USER>` | ✅ |
| DocumentsLocation | `<USER>` | `<USER>` | ✅ |
| FontsLocation | none | none | ✅ |
| ApplicationsLocation | `<USER>` | `<USER>` | ✅ |
| MusicLocation | `<USER>` | `<USER>` | ✅ |
| MoviesLocation | `<USER>` | `<USER>` | ✅ |
| PicturesLocation | `<USER>` | `<USER>` | ✅ |
| TempLocation | `<USER>` | `<USER>` | ✅ |
| HomeLocation | `<USER>` | `<USER>` | ✅ |
| AppLocalDataLocation | `<USER>`, `<APPNAME>`, `<APPDIR>` | `<USER>`, `<ORG/APP>`, `<APPDIR>` | ✅ |
| CacheLocation | `<USER>`, `<APPNAME>` | `<USER>`, `<ORG/APP>` | ✅ |
| GenericDataLocation | `<USER>`, `<APPDIR>` | `<USER>`, `<APPDIR>` | ✅ |
| RuntimeLocation | `<USER>` | `<USER>` | ✅ |
| **ConfigLocation** | `<USER>`, `<APPNAME>` | `<USER>`, `<ORG/APP>`, `<APPDIR>` | ⚠️ Missing `<APPDIR>` in docs |
| DownloadLocation | `<USER>` | `<USER>` | ✅ |
| GenericCacheLocation | `<USER>` | `<USER>` | ✅ |
| **GenericConfigLocation** | `<USER>` | `<USER>`, `<APPDIR>` | ⚠️ Missing `<APPDIR>` in docs |
| AppDataLocation | `<USER>`, `<APPNAME>`, `<APPDIR>` | `<USER>`, `<ORG/APP>`, `<APPDIR>` | ✅ |
| **AppConfigLocation** | `<USER>`, `<APPNAME>` | `<USER>`, `<ORG/APP>`, `<APPDIR>` | ⚠️ Missing `<APPDIR>` in docs |
| PublicShareLocation | none | none | ✅ |
| TemplatesLocation | `<USER>` | `<USER>` | ✅ |

### Important Discrepancies Found

1. **ConfigLocation** - Documentation doesn't mention `<APPDIR>` but it's present in actual paths
2. **GenericConfigLocation** - Documentation doesn't mention `<APPDIR>` but it's present in actual paths  
3. **AppConfigLocation** - Documentation doesn't mention `<APPDIR>` but it's present in actual paths

**Pattern**: All "Config" location types include `<APPDIR>` as fallback paths, even though Qt documentation doesn't explicitly mention this.

### Organization/Application Name Behavior

#### When Both Are Set
```cpp
app.setOrganizationName("jartisan");
app.setApplicationName("OpenSCAD");
```
**Result**: `C:/Users/Jeff/AppData/Local/jartisan/OpenSCAD`

Format is: `Organization/ApplicationName` (directory hierarchy)

#### When Only Application Name Is Set
```cpp
app.setOrganizationName("");
app.setApplicationName("OpenSCAD");
```
**Result**: `C:/Users/Jeff/AppData/Local/OpenSCAD`

No organization folder level.

#### When Only Organization Name Is Set
```cpp
app.setOrganizationName("jartisan");
app.setApplicationName("");
```
**Result**: `C:/Users/Jeff/AppData/Local/jartisan/appname_expansion_test`

Uses executable name as fallback for application name.

#### When Neither Is Set
```cpp
app.setOrganizationName("");
app.setApplicationName("");
```
**Result**: `C:/Users/Jeff/AppData/Local/appname_expansion_test`

Uses executable name only.

### Key Insights

1. **`<USER>` expansion** means the Windows username appears in the path
2. **`<ORG/APP>` expansion** means organization and/or app name in the path (two-level directory structure)
3. **`<APPDIR>` expansion** means executable directory appears in the path (for portable/fallback scenarios)
4. **GenericDataLocation** correctly does NOT expand `<APPNAME>` (it's generic!)
5. **App-Specific locations** (App*) correctly expand `<APPNAME>` as `<ORG>/<APP>`
6. **All Config locations** include `<APPDIR>` fallback paths despite documentation not mentioning it

### Documentation Issues

The Qt documentation's use of `<APPNAME>` is **misleading**. It should really be:
- `<ORGNAME>/<APPNAME>` for paths with both organization and application
- The organization and application names are kept **separate** as directory levels

## Example Output

Running `appname_expansion_test.exe`:

```
DesktopLocation                Expected: <USER>
    Actual:   <USER>
    [1] C:/Users/Jeff/Desktop

AppDataLocation                Expected: <USER>, <APPNAME>, <APPDIR>
    Actual:   <USER>, <ORG/APP>, <APPDIR>
    [1] C:/Users/Jeff/AppData/Roaming/jartisan/OpenSCAD
    [2] C:/ProgramData/jartisan/OpenSCAD
    [3] D:/repositories/cppsnippets/cppsnippets/QtExploration/build/bin/Debug
    ... (2 more paths)

GenericDataLocation            Expected: <USER>, <APPDIR>
    Actual:   <USER>, <APPDIR>
    [1] C:/Users/Jeff/AppData/Local
    [2] C:/ProgramData
    [3] D:/repositories/cppsnippets/cppsnippets/QtExploration/build/bin/Debug
    ... (1 more paths)
```

## Notes on findExecutable()

`QStandardPaths::findExecutable()` **ONLY searches the PATH** environment variable.

It will NOT find OpenSCAD.exe if:
- OpenSCAD is installed in `C:\Program Files\OpenSCAD\`
- But `C:\Program Files\OpenSCAD\` is NOT in your PATH

This is why the demo has three different approaches:

1. `findExecutable(appname)` - Searches system PATH only
2. `findExecutable(appname, paths)` - Searches specific directories you provide (but still doesn't recurse)
3. `findOpenSCADDirectories()` with `QDirListing` - Recursively searches for directories named "OpenSCAD"

For finding installed applications like OpenSCAD that aren't in PATH, you need:
- Windows Registry (for proper application discovery)
- Recursive directory search (like the QDirListing approach)
- Known installation paths (check `C:\Program Files\OpenSCAD`, `C:\Program Files (x86)\OpenSCAD`, etc.)

`findExecutable()` is best for finding command-line tools that users intentionally added to PATH (like `git`, `python`, `node`, etc.), not GUI applications.

## References

- [Qt 6.10 QStandardPaths Documentation](https://doc.qt.io/qt-6/qstandardpaths.html)
- [../doc/stdpathstable.md](../doc/stdpathstable.md) - Comprehensive table of all StandardLocation types

