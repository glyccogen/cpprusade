add_executable(compressed_pair
        test.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/catch_main.cpp
)

target_include_directories(compressed_pair PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/.for_tests
)
