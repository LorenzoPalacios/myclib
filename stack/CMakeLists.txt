# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(stack)

function(init_lib_stack target)
  target_sources(${target}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/stack.h"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/stack.c"
  )
endfunction()

add_library(stack STATIC)
init_lib_stack(stack)
