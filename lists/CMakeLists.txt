# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(lists)

set(SINGLY_DIR "${PROJECT_SOURCE_DIR}/singly")

function(init_lib_singly target)
  target_sources(${target}
    PUBLIC
    "${SINGLY_DIR}/sll.h"
    PRIVATE
    "${SINGLY_DIR}/sll.c"
  )
endfunction()

add_library(lists STATIC)
init_lib_singly(lists)
