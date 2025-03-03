# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(vector)

function(init_lib_random target)
  target_sources(${target}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/vector.h"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/vector.c"
  )
endfunction()

add_library(vector STATIC)
init_lib_random(vector)
