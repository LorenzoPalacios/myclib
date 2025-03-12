# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(myclib)

set(BUILD_TESTS OFF)

set(DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_compile_options(
  -O3 -pedantic -Wabi -Wall -Wcast-align -Wcast-qual -Wconversion -Werror
  -Wextra -Wformat=2 -Wshadow -Wstrict-overflow=5 -Wundef -Wunreachable-code
  -Wwrite-strings -march=native -mtune=native
)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR true)

add_subdirectory("random")
add_subdirectory("stack")
add_subdirectory("str")
# add_subdirectory("trees")
add_subdirectory("vector")

target_include_directories(random PUBLIC "include")
target_include_directories(stack PUBLIC "include")
target_include_directories(str PUBLIC "include")
# target_include_directories(trees PUBLIC "include")
target_include_directories(vector PUBLIC "include")

if (BUILD_TESTS)
set(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests")
set(STACK_TEST_DIR "${TESTS_DIR}/teststack")

function(init_stack_test target)
  target_sources(${target}
    PUBLIC
    "${STACK_TEST_DIR}/teststack.h"
    PRIVATE
    "${STACK_TEST_DIR}/teststack.c"
  )
endfunction()

add_executable(tests "${TESTS_DIR}/main.c")
target_link_libraries(tests stack)
init_stack_test(tests)
endif()
