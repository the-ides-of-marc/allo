cmake_minimum_required(VERSION 3.25)

project(allo
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Enable UBSan and ASan" OFF)

include(GNUInstallDirs)

function(setup_dev_flags target_name)
    set_target_properties(${target_name} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    target_compile_options(${target_name}
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:
                -Wall
                -Wextra
                -Werror
                -Wpedantic
                -Wconversion
                -Wshadow
                -Wformat=2
                -Wimplicit-fallthrough
                -Wvla
                $<$<CONFIG:Debug>:-g>
                $<$<BOOL:${ENABLE_SANITIZERS}>:
                    -fsanitize=address,undefined
                    -fno-omit-frame-pointer
                    -g
                >
            >
    )
    target_link_options(${target_name}
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:
                $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
            >
    )
endfunction()


add_library(allo)
add_library(allo::allo ALIAS allo)
target_sources(allo
    PRIVATE
        src/bump.c
        src/stack.c
        src/pool.c
)
target_include_directories(allo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        src
)
setup_dev_flags(allo)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_subdirectory(tests)
endif()


install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(TARGETS allo
    EXPORT allo-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(EXPORT allo-targets
    FILE alloTargets.cmake
    NAMESPACE allo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/allo
)
