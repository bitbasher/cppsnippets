# Application Metadata - Single Source of Truth
# Edit this file to change app name, author, organization
# Command-line overrides: cmake -DAPP_NAME="MyApp" -DAPP_SUFFIX="Nightly" ..

set(APP_NAME "ScadTemplates" CACHE STRING "Application name")
set(APP_SUFFIX "" CACHE STRING "Application suffix (e.g., 'Nightly', 'Beta')")
set(APP_AUTHOR "jartisan" CACHE STRING "Application author")
set(APP_ORGANIZATION "CppSnippets" CACHE STRING "Organization name")

# Version numbers
set(APP_VERSION_MAJOR 2 CACHE STRING "Major version number")
set(APP_VERSION_PATCH 0 CACHE STRING "Patch version number")
# VERSION_MINOR is auto-calculated from git commit count
