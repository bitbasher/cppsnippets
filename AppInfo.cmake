# Application Metadata - Single Source of Truth
# Edit this file to change app name, author, organization
# Command-line overrides: cmake -DAPP_NAME="MyApp" -DAPP_SUFFIX="Nightly"
#  NB suffixes are appended to the appname by "_", e.g. "MyApp_Nightly"
#   and to appFileName by " " and in parens, e.g. "MyApp (Nightly)"
#   and to appDisplayName by " ", e.g. "MyApp Nightly"

set(APP_NAME "OpenSCAD" CACHE STRING "Application name")
set(APP_SUFFIX "" CACHE STRING "Suffix to mark special versions (e.g., 'Nightly', 'Beta')")
set(APP_AUTHOR "Jeff Hayes" CACHE STRING "Application author")
set(APP_ORGANIZATION "jartisan" CACHE STRING "Organization name")

# Version numbers
set(APP_VERSION_MAJOR 2 CACHE STRING "Major version number")
# VERSION_MINOR is auto-calculated from git commit count
set(APP_VERSION_PATCH 0 CACHE STRING "Patch version number")
