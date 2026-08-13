add_executable(shared_from_this
        test.cpp
        test_shared.cpp
        test_weak.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/allocations_checker.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/catch_main.cpp
)

target_include_directories(shared_from_this PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/.for_tests
)
