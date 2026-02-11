function(add_allo_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.c)
    target_link_libraries(${TEST_NAME}
        PRIVATE
            dev_config
            allo
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()


set(TESTS
    test_interface
)

foreach(TEST ${TESTS})
    add_allo_test(${TEST})
endforeach()
