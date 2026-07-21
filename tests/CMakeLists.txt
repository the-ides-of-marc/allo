include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.1
    SYSTEM
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(unity)

add_library(allo_test STATIC
    allo_test/allo_test_assert.c
    allo_test/allo_test_io.c
    allo_test/allo_test_mem.c
)
target_include_directories(allo_test PUBLIC allo_test)
target_link_libraries(allo_test PRIVATE unity allo)

function(add_allo_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    target_link_libraries(${TEST_NAME}
        PRIVATE
            unity
            allo
            allo_test
    )
    target_compile_definitions(${TEST_NAME} PRIVATE ALLO_ENABLE_ASSERT)
    setup_dev_flags(${TEST_NAME})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME}
        PROPERTIES
            ENVIRONMENT
            "ASAN_OPTIONS=detect_leaks=1:symbolize=1:abort_on_error=1"
        )
endfunction()


set(TESTS
    test_common
    test_math
    test_allocator
    test_bump
    test_stack
    test_pool
)

foreach(TEST ${TESTS})
    add_allo_test(${TEST})
endforeach()
