# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(tests)

set(STACK_TESTS_DIR "${PROJECT_SOURCE_DIR}/stacktests")
set(VECTOR_TESTS_DIR "${PROJECT_SOURCE_DIR}/vectortests")
set(FRAMEWORK_DIR "${PROJECT_SOURCE_DIR}")

function(init_framework target)
  target_sources(${target}
    PUBLIC
    "${FRAMEWORK_DIR}/framework.h"
    PRIVATE
    "${FRAMEWORK_DIR}/framework.c"
    "${FRAMEWORK_DIR}/main.c"
  )
endfunction()

function(init_stack_tests target)
  target_sources(${target}
    PUBLIC
    "${STACK_TESTS_DIR}/stacktests.h"
    PRIVATE
    "${STACK_TESTS_DIR}/stacktests.c"
  )
endfunction()

function(init_vector_tests target)
  target_sources(${target}
    PUBLIC
    "${VECTOR_TESTS_DIR}/vectortests.h"
    PRIVATE
    "${VECTOR_TESTS_DIR}/vectortests.c"
  )
endfunction()

add_executable(tests)
init_framework(tests)
init_stack_tests(tests)
init_vector_tests(tests)

target_link_libraries(tests stack)
target_link_libraries(tests vector)
