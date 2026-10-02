cmake_minimum_required(VERSION 3.25)
project(tradevault VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Forces standard C++ instead of compiler dialect
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# One place to define warning settings, written portably.
add_library(tradevault_warnings INTERFACE)

if(MSVC) # Microsoft's compiler uses different flag syntax than GCC/Clang
    target_compile_options(tradevault_warnings INTERFACE /W4 /permissive- /utf-8)
else()
    target_compile_options(tradevault_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

# Third-party dependencies installed through vcpkg.
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

# Temporary PostgreSQL connection smoke test.
add_executable(db_smoke
    app/db_smoke.cpp
)

target_link_libraries(db_smoke
    PRIVATE
        libpqxx::pqxx
        tradevault_warnings
)

add_subdirectory(src)
add_subdirectory(tests)