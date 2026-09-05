cmake_minimum_required(VERSION 3.25)
project(tradevault VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #Forces standard C++ instead of compiler dialect
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# One place to define warning settings, written portably.
add_library(tradevault_warnings INTERFACE)
if(MSVC) #Exists as microsofts compiler and gcc use different flag syntax 
    target_compile_options(tradevault_warnings INTERFACE /W4 /permissive- /utf-8)
else()
    target_compile_options(tradevault_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

find_package(spdlog CONFIG REQUIRED) #Searches for the vcpkg installed libraries, failing loud if they are missing
find_package(GTest CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)