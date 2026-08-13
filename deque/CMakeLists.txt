add_executable(deque
        test.cpp
        ${PROJECT_SOURCE_DIR}/.for_tests/catch_main.cpp
)

target_compile_options(deque PRIVATE -Wno-self-assign-overloaded)

target_include_directories(deque PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/.for_tests
)
