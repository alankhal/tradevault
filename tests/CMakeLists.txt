add_executable(core_tests
    version_test.cpp
)

target_link_libraries(core_tests
    PRIVATE tradevault_core tradevault_warnings GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)