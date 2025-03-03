# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(random)

function(init_lib_random target)
  target_sources(${target}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/random.h"
    PRIVATE
    "${PROJECT_SOURCE_DIR}/random.c"
  )
endfunction()

add_library(random STATIC)
init_lib_random(random)
