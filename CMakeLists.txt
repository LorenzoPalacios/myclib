# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(myclib)

set(BUILD_TESTS ON)

set(CMAKE_C_STANDARD 90) # This can be freely adjusted.
set(CMAKE_COMPILE_WARNING_AS_ERROR true)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DEBUG_BUILD OFF)

# Compiler Arguments
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-qual -Wconversion -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wfloat-equal -Wformat=2 -Winit-self")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline -Wlogical-op -Wpedantic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wredundant-decls")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wstrict-overflow=5")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-aliasing=2 -Wswitch-default")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunreachable-code")

    if(DEBUG_BUILD)
        set(CMAKE_C_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    else()
        set(CMAKE_C_FLAGS "-D_FORTIFY_SOURCE=2 -O2")
    endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything -Wno-c23-compat")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-cast-align -Wno-switch-enum")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-padded")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unsafe-buffer-usage")

    if(DEBUG_BUILD)
        set(CMAKE_C_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    else()
        set(CMAKE_C_FLAGS "-D_FORTIFY_SOURCE=2 -O2")
    endif()

    if(CMAKE_C_STANDARD GREATER_EQUAL 11)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pre-c11-compat")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-declaration-after-statement")
    endif()

    if(CMAKE_C_STANDARD GREATER_EQUAL 23)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pre-c23-compat")
    endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_C_FLAGS "/analyze /guard:cf /Gw /MP /Ox /Wall /WL")
endif()

# Building Library

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}")

set(RANDOM_DIR "${PROJECT_SOURCE_DIR}/random")
set(STACK_DIR "${PROJECT_SOURCE_DIR}/stack")
set(STR_DIR "${PROJECT_SOURCE_DIR}/str")
set(VECTOR_DIR "${PROJECT_SOURCE_DIR}/vector")

add_library(myclib STATIC)
target_include_directories(myclib PUBLIC ${INCLUDE_DIR})

target_sources(myclib
    PUBLIC "${RANDOM_DIR}/random.h"
    PRIVATE "${RANDOM_DIR}/random.c")
target_sources(myclib
    PUBLIC "${STACK_DIR}/stack.h"
    PRIVATE "${STACK_DIR}/stack.c")
target_sources(myclib
    PUBLIC "${STR_DIR}/str.h"
    PRIVATE "${STR_DIR}/str.c")
target_sources(myclib PUBLIC "${VECTOR_DIR}/vector.h")

# Building Tests

if(BUILD_TESTS)
    set(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests")
    set(STACKTESTS_DIR "${TESTS_DIR}/stacktests")
    set(STRTESTS_DIR "${TESTS_DIR}/strtests")
    set(VECTORTESTS_DIR "${TESTS_DIR}/vectortests")

    add_executable(tests)
    target_sources(tests
        PRIVATE
        "${TESTS_DIR}/main.c" "${TESTS_DIR}/framework.c"
        "${STACKTESTS_DIR}/stacktests.c"
        "${STRTESTS_DIR}/strtests.c"
        "${VECTORTESTS_DIR}/vectortests.c"
        PUBLIC
        "${TESTS_DIR}/framework.h"
        "${STACKTESTS_DIR}/stacktests.h"
        "${STRTESTS_DIR}/strtests.h"
        "${VECTORTESTS_DIR}/vectortests.h"
    )
    add_dependencies(tests myclib)
    target_link_libraries(tests PUBLIC myclib)
endif()
