# VS Code IntelliSense Configuration

**AGENT NOTES FOR VS CODE INTELLISENSE:**

File: `.vscode/c_cpp_properties.json`

**Key Settings:**
- `configurationProvider`: `ms-vscode.cmake-tools` (official Microsoft extension)
- Automatically discovers Qt headers and all project includes from CMake
- No need to manually list include paths for external libraries

**Architecture:**
- Platform: Windows MSVC + Visual Studio 17 2022 generator
- NOT Clang, NOT Ninja, NOT Unix generators
- See DEVELOPMENT.md section 2 for why

**PowerShell Environment:**
- This is Windows PowerShell, NOT Bash
- Avoid Unix commands: head, tail, grep, wc
- Use PowerShell equivalents: Select-Object -First N / -Last N
- See DEVELOPMENT.md section 4 for full list

---

## Build Variants (Debug & Release)

File: `.vscode/settings.json`

**CMake Variants Configured:**
- `debug` (default) — Debug build with symbols for debugging
- `release` — Release build optimized for performance

**How to use:**
1. In VS Code, look for the **Build variant selector** in the CMake toolbar
2. Click to switch between "Debug" and "Release"
3. The toolbar will rebuild with the selected configuration
4. You can also build/test for each variant independently

**What this does:**
- Visual Studio 17 2022 is a multi-config generator
- Variants allow switching between Debug/Release at build time
- Same binary configuration exists in the build tree (no need to reconfigure)

---

## Troubleshooting

**If Qt headers show red squiggles:**
1. Rebuild: `cmake --build .`
2. Reload VS Code: Ctrl+K Ctrl+R
3. Ensure ms-vscode.cmake-tools extension is installed and active

**If generic includes still missing:**
1. Verify CMakeLists.txt lines 3-6 (job pool configuration)
2. Run: `cmake --build .`
3. Reload VS Code

**If Build variant selector doesn't appear:**
1. Reload VS Code
2. Check settings.json has `cmake.variants` (added December 21, 2025)
3. Run CMake Configure (F7) to refresh

See DEVELOPMENT.md section 3 for detailed troubleshooting.
