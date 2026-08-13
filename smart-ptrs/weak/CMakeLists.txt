add_executable(weak
        test.cpp
        test_shared.cpp
        test_odr.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/allocations_checker.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/catch_main.cpp
)

target_include_directories(weak PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/.for_tests
)
