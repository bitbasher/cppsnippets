# QStandardPaths StandardLocation Reference

## Column Index

- **Path type** - The human-readable name (e.g., DesktopLocation)
- **Enum Value** - The integer enum value (0-22)
- **Type** - Location category:
  - **AS** = Application-Specific (uses organization/application name)
  - **G** = Generic (system-wide or user directories, no app name)
  - **Mixed** = Behavior varies by platform
- **Scope** - What the location represents:
  - **User** = User-specific directory (documents, desktop, etc.)
  - **App** = Application-specific directory (uses `<APPNAME>`)
  - **Sys** = System-wide directory (shared across users/apps)
  - **Pub** = Public/shared directory
- **NE** - Path guaranteed Non-Empty (always returns at least one path)
- **macOS** - Typical paths on macOS
- **Windows** - Typical paths on Windows
- **Linux/Posix** - Typical paths on Linux/Unix
- **Qt Vers.** - Qt version when location was introduced (blank = pre-5.4)
- **Description** - Detailed explanation of the location's purpose

## Placeholder Tags in Paths

Qt uses placeholder tags in path templates that get replaced at runtime:

### Tag Reference

- **`<USER>`** - Replaced with the system username
  - Example: `C:/Users/<USER>/Desktop` → `C:/Users/Jeff/Desktop`
  
- **`<APPNAME>`** - Replaced with organization and application name as **two-level directory structure**
  - Example: `C:/Users/<USER>/AppData/Local/<APPNAME>` → `C:/Users/Jeff/AppData/Local/jartisan/OpenSCAD`
  - Format: `<ORGNAME>/<APPNAME>` (separate directory levels, not a single combined string)
  - If only app name set: uses app name only
  - If only org name set: uses org + executable name
  - If neither set: uses executable name only
  
- **`<APPDIR>`** - Replaced with the executable's directory (for portable/fallback paths)
  - Example: `<APPDIR>/data` → `D:/repositories/cppsnippets/build/bin/Debug/data`
  - Used for development and portable application scenarios

### Setting Organization/Application Names in C++

```cpp
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set organization name (first directory level)
    app.setOrganizationName("jartisan");
    
    // Set application name (second directory level)
    app.setApplicationName("OpenSCAD");
    
    // QStandardPaths will now use these values for <APPNAME> expansion
    // Result: paths like "C:/Users/Jeff/AppData/Local/jartisan/OpenSCAD"
    
    return app.exec();
}
```

### Important: Documentation Discrepancies

Testing revealed that **3 Config-related locations** have undocumented `<APPDIR>` usage:

1. **ConfigLocation** - Docs say `<USER>`, `<APPNAME>` but actual paths also include `<APPDIR>`
2. **GenericConfigLocation** - Docs say `<USER>` only but actual paths also include `<APPDIR>`
3. **AppConfigLocation** - Docs say `<USER>`, `<APPNAME>` but actual paths also include `<APPDIR>`

All Config location types include `<APPDIR>` as fallback paths even though Qt documentation doesn't explicitly mention this.

See [QtExploration/README.md](../QtExploration/README.md) for comprehensive test results comparing expected vs actual tag expansion.

---

## StandardLocation Table

| Path type | Enum Value | Type | Scope | NE | macOS | Windows | Linux/Posix | Qt Vers. | Description  |
|-----|--|--|-----|--|------|-----|-------|-----|------------------ |
| DesktopLocation | 0 | G | User | | "~/Desktop" | "C:/Users/&lt;USER&gt;/Desktop" | ~/Desktop" | | User's desktop directory. On systems with no concept of a desktop, same as HomeLocation. |
| DocumentsLocation | 1 | G | User | NE | "~/Documents" | "C:/Users/&lt;USER&gt;/Documents" | ~/Documents" | | Directory containing user document files. |
| FontsLocation | 2 | G | Sys | | "/System/Library/Fonts" (not writable) | "C:/Windows/Fonts" (not writable) | ~/.fonts", "~/.local/share/fonts", "/usr/local/share/fonts", "/usr/share/fonts" | | Directory containing user's fonts. Installing fonts may require additional platform-specific operations. |
| ApplicationsLocation | 3 | G | Sys | | "/Applications" (not writable) | "C:/Users/&lt;USER&gt;/AppData/Roaming/Microsoft/Windows/Start Menu/Programs" | ~/.local/share/applications", "/usr/local/share/applications", "/usr/share/applications" | | Directory containing user applications (executables, bundles, or shortcuts). Installing applications may require additional platform-specific operations. |
| MusicLocation | 4 | G | User | | "~/Music" | "C:/Users/&lt;USER&gt;/Music" | ~/Music" | | Directory containing user's music or audio files. Falls back to documents directory if no music-specific directory exists. |
| MoviesLocation | 5 | G | User | | "~/Movies" | "C:/Users/&lt;USER&gt;/Videos" | ~/Videos" | | Directory containing user's movies and videos. Falls back to documents directory if no movies-specific directory exists. |
| PicturesLocation | 6 | G | User | | "~/Pictures" | "C:/Users/&lt;USER&gt;/Pictures" | ~/Pictures" | | Directory containing user's pictures or photos. Falls back to documents directory if no pictures-specific directory exists. |
| TempLocation | 7 | Mixed | Sys | NE | randomly generated by the OS | "C:/Users/&lt;USER&gt;/AppData/Local/Temp" | /tmp" | | Directory for temporary files. May be app-specific, shared among apps, or system-wide. |
| HomeLocation | 8 | Mixed | User | NE | "~" | "C:/Users/&lt;USER&gt;" | ~" | | User's home directory (same as QDir::homePath()). On Unix, equals HOME environment variable. May be generic or app-specific. |
| AppLocalDataLocation | 9 | AS | App | | "~/Library/Application Support/&lt;APPNAME&gt;", "/Library/Application Support/&lt;APPNAME&gt;". "&lt;APPDIR&gt;/../Resources" | "C:/Users/&lt;USER&gt;/AppData/Local/&lt;APPNAME&gt;", "C:/ProgramData/&lt;APPNAME&gt;", "&lt;APPDIR&gt;", "&lt;APPDIR&gt;/data", "&lt;APPDIR&gt;/data/&lt;APPNAME&gt;" | ~/.local/share/&lt;APPNAME&gt;", "/usr/local/share/&lt;APPNAME&gt;", "/usr/share/&lt;APPNAME&gt;" | 5.4 | Local settings path on Windows. On other platforms, same as AppDataLocation. |
| CacheLocation | 10 | AS | App | NE | "~/Library/Caches/&lt;APPNAME&gt;", "/Library/Caches/&lt;APPNAME&gt;" | "C:/Users/&lt;USER&gt;/AppData/Local/&lt;APPNAME&gt;/cache" | ~/.cache/&lt;APPNAME&gt;" | | Application-specific directory for non-essential cached data. |
| GenericDataLocation | 11 | G | Sys | NE | "~/Library/Application Support", "/Library/Application Support" | "C:/Users/&lt;USER&gt;/AppData/Local", "C:/ProgramData", "&lt;APPDIR&gt;", "&lt;APPDIR&gt;/data" | ~/.local/share", "/usr/local/share", "/usr/share" | | Directory for persistent data shared across applications. |
| RuntimeLocation | 12 | G | Sys | | "~/Library/Application Support" | "C:/Users/&lt;USER&gt;" | /run/user/&lt;USER&gt;" | | Directory for runtime communication files like Unix local sockets. Path may be empty on some systems. |
| ConfigLocation | 13 | Mixed | App | NE | "~/Library/Preferences" | "C:/Users/&lt;USER&gt;/AppData/Local/&lt;APPNAME&gt;", "C:/ProgramData/&lt;APPNAME&gt;" | ~/.config", "/etc/xdg" | | Directory for user-specific configuration files. May be generic or app-specific. |
| DownloadLocation | 14 | G | User | | "~/Downloads" | "C:/Users/&lt;USER&gt;/Downloads" | ~/Downloads" | | Directory for user's downloaded files. Falls back to documents directory if no downloads-specific directory exists. |
| GenericCacheLocation | 15 | G | Sys | | "~/Library/Caches", "/Library/Caches" | "C:/Users/&lt;USER&gt;/AppData/Local/cache" | ~/.cache" | | Directory for non-essential cached data shared across applications. Path may be empty if system has no shared cache concept. |
| GenericConfigLocation | 16 | G | Sys | NE | "~/Library/Preferences" | "C:/Users/&lt;USER&gt;/AppData/Local", "C:/ProgramData" | ~/.config", "/etc/xdg" | | Directory for configuration files shared between multiple applications. |
| AppDataLocation | 17 | AS | App | NE | "~/Library/Application Support/&lt;APPNAME&gt;", "/Library/Application Support/&lt;APPNAME&gt;". "&lt;APPDIR&gt;/../Resources" | "C:/Users/&lt;USER&gt;/AppData/Roaming/&lt;APPNAME&gt;", "C:/ProgramData/&lt;APPNAME&gt;", "&lt;APPDIR&gt;", "&lt;APPDIR&gt;/data", "&lt;APPDIR&gt;/data/&lt;APPNAME&gt;" | ~/.local/share/&lt;APPNAME&gt;", "/usr/local/share/&lt;APPNAME&gt;", "/usr/share/&lt;APPNAME&gt;" | 5.4 | Application-specific directory for persistent data. On Windows, returns roaming path. Use GenericDataLocation for shared data. |
| AppConfigLocation | 18 | AS | App | NE | "~/Library/Preferences/&lt;APPNAME&gt;" | "C:/Users/&lt;USER&gt;/AppData/Local/&lt;APPNAME&gt;", "C:/ProgramData/&lt;APPNAME&gt;" | ~/.config/&lt;APPNAME&gt;", "/etc/xdg/&lt;APPNAME&gt;" | 5.5 | Application-specific directory for configuration files. |
| PublicShareLocation | 19 | G | Pub | | "~/Public" | "C:/Users/Public" | ~/Public" | 6.4 | Directory for user-specific publicly shared files and directories can be stored. This is a generic value. Note that the returned path may be empty if the system has no concept of a publicly shared location. |
| TemplatesLocation | 20 | G | User | | "~/Templates" | "C:/Users/&lt;USER&gt;/AppData/Roaming/Microsoft/Windows/Templates" | ~/Templates" | 6.4 | Directory for template files. Path may be empty if system has no templates concept. |
| StateLocation | 21 | AS | App | NE | "~/Library/Preferences/&lt;APPNAME&gt;/State" | "C:/Users/&lt;USER&gt;/AppData/Local/&lt;APPNAME&gt;/State", "C:/ProgramData/&lt;APPNAME&gt;/State" | ~/.local/state/&lt;APPNAME&gt;" | 6.7 | Application-specific directory for state data files. |
| GenericStateLocation | 22 | Mixed | Sys | NE | "~/Library/Preferences/State" | "C:/Users/&lt;USER&gt;/AppData/Local/State", "C:/ProgramData/&lt;APPNAME&gt;/State" | ~/.local/state" | 6.7 | Directory for shared state data files across applications. May be generic or app-specific. |
