# CMakeLists.txt for TabDialog example
# Demonstrates Qt TabDialog pattern where each tab is its own QWidget subclass

cmake_minimum_required(VERSION 3.16)
project(tabdialog_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic handling of MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 (or Qt5 as fallback)
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Tab widget source files
set(TAB_SOURCES
    generalTab.cpp
    permsTab.cpp
    appsTab.cpp
    tabDialog.cpp
    tabDialogMain.cpp
)

set(TAB_HEADERS
    generalTab.hpp
    permsTab.hpp
    appsTab.hpp
    tabDialog.hpp
)

# Create the executable
add_executable(tabdialog_example
    ${TAB_SOURCES}
    ${TAB_HEADERS}
)

# Link Qt libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(tabdialog_example PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
else()
    target_link_libraries(tabdialog_example PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )
endif()

# Set output directory
set_target_properties(tabdialog_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
