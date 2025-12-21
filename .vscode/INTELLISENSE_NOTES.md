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

**Troubleshooting:**
1. If Qt headers show red squiggles:
   - Rebuild: `cmake --build .`
   - Reload VS Code: Ctrl+K Ctrl+R
   - Ensure ms-vscode.cmake-tools extension is installed and active

2. If generic includes still missing:
   - Verify CMakeLists.txt lines 3-6 (job pool configuration)
   - Run: `cmake --build .`
   - Reload VS Code

See DEVELOPMENT.md section 3 for detailed troubleshooting.
