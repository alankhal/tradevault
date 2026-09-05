add_library(tradevault_core
    core/version.cpp
)

target_include_directories(tradevault_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tradevault_core
    PUBLIC  spdlog::spdlog
    PRIVATE tradevault_warnings
)

add_executable(tradevault_api main.cpp)
target_link_libraries(tradevault_api PRIVATE tradevault_core tradevault_warnings)