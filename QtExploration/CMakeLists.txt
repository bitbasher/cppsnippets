cmake_minimum_required(VERSION 3.16)
project(QtExploration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.1/msvc2022_64")
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${QT6_DEFAULT_PATH})
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 1. locate() demo
add_executable(locate_demo locate_demo.cpp)
target_link_libraries(locate_demo PRIVATE Qt6::Core)
set_target_properties(locate_demo PROPERTIES
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME "locate_demo"
)

# 2. locateAll() demo
add_executable(locateall_demo locateall_demo.cpp)
target_link_libraries(locateall_demo PRIVATE Qt6::Core)
set_target_properties(locateall_demo PROPERTIES
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME "locateall_demo"
)

# 3. findExecutable() demo
add_executable(findexecutable_demo findexecutable_demo.cpp)
target_link_libraries(findexecutable_demo PRIVATE Qt6::Core)
set_target_properties(findexecutable_demo PROPERTIES
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME "findexecutable_demo"
)

# 4. standardLocations() demo
add_executable(standardlocations_demo standardlocations_demo.cpp)
target_link_libraries(standardlocations_demo PRIVATE Qt6::Core)
set_target_properties(standardlocations_demo PROPERTIES
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME "standardlocations_demo"
)

# 5. appname_expansion_test
add_executable(appname_expansion_test appname_expansion_test.cpp)
target_link_libraries(appname_expansion_test PRIVATE Qt6::Core)
set_target_properties(appname_expansion_test PROPERTIES
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME "appname_expansion_test"
)

# Copy Qt DLLs on Windows
if(WIN32)
    add_custom_command(TARGET locate_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt DLLs..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE_DIR:locate_demo>
    )
endif()
