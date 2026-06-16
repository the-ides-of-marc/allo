cmake_minimum_required(VERSION 3.25)

project(allo
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Enable UBSan and ASan" OFF)

include(GNUInstallDirs)

function(setup_dev_flags target_name)
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
        src/allo_bump.c
        src/allo_pool.c
)
target_include_directories(allo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        src
)
setup_dev_flags(allo)
target_compile_features(allo PUBLIC c_std_99)

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
