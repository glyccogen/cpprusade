add_executable(unique
        test.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/catch_main.cpp
)

target_include_directories(unique PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/.for_tests
)
