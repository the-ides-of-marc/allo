include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(unity)

add_library(test_utils STATIC test_utils.c)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_utils PRIVATE unity)

function(add_allo_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    target_link_libraries(${TEST_NAME}
        PRIVATE
            dev_config
            allo
            unity
            test_utils
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME}
        PROPERTIES
            ENVIRONMENT
            "ASAN_OPTIONS=detect_leaks=1:symbolize=1:abort_on_error=1"
        )
endfunction()


set(TESTS
    test_allocator
    test_bump
    test_pool
)

foreach(TEST ${TESTS})
    add_allo_test(${TEST})
endforeach()
