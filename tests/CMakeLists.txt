#Core C++ Logic Testing 
add_executable(core_tests
    version_test.cpp
    repository_test.cpp
    validator_test.cpp 
    service_test.cpp
)

target_link_libraries(core_tests
    PRIVATE tradevault_core tradevault_warnings GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)

#API Testing 
add_executable(api_tests
    controller_test.cpp

    ${PROJECT_SOURCE_DIR}/src/api/TradeController.cpp
    ${PROJECT_SOURCE_DIR}/src/api/TradeJson.cpp
    ${PROJECT_SOURCE_DIR}/src/api/HttpErrorMapper.cpp
)

target_link_libraries(api_tests
    PRIVATE
        tradevault_core
        tradevault_warnings
        Drogon::Drogon
        GTest::gtest_main
)

gtest_discover_tests(api_tests)