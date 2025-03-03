# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(str)

function(init_lib_str target)
  target_sources(${target}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/str.h"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/str.c"
  )
endfunction()

add_library(str STATIC)
init_lib_str(str)
