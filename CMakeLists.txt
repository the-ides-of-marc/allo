cmake_minimum_required(VERSION 3.15)

project(allo
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(GNUInstallDirs)

add_library(dev_config INTERFACE)

target_compile_options(dev_config INTERFACE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wformat=2
    -Wimplicit-fallthrough
    -Wvla
    $<$<CONFIG:Debug>:
        -fsanitize=address,undefined;
        -fno-omit-frame-pointer;
        -g
    >
)
target_link_options(dev_config INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

add_library(allo INTERFACE)
add_library(allo::allo ALIAS allo)
target_include_directories(allo INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(allo INTERFACE
    $<BUILD_INTERFACE:dev_config>
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_subdirectory(tests)
endif()


install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(TARGETS allo
    EXPORT allo-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT allo-targets
    FILE alloTargets.cmake
    NAMESPACE allo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/allo
)
