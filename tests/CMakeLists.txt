# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(tests)

set(STACK_TEST_DIR "${PROJECT_SOURCE_DIR}/teststack")

function(init_stack_test target)
  target_sources(${target}
    PUBLIC
    "${STACK_TEST_DIR}/teststack.h"
    PRIVATE
    "${STACK_TEST_DIR}/teststack.c"
  )
endfunction()

add_executable(tests)
init_stack_test(tests)
