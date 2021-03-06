cmake_minimum_required(VERSION 3.12)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project("httpc")

enable_testing()

set(SOURCES src/httpC-cli.c)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS "${warnings} -lcurl" CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

add_subdirectory(src)
add_subdirectory(test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions( -DCURL_STATICLIB )

add_executable(httpc ${SOURCES})

target_link_libraries(httpc curl)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL "A flag showing that CMake has configured at least once.")
