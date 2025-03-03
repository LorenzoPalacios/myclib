# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(trees)

set(BINARYTREE_DIR "${PROJECT_SOURCE_DIR}/binarytree")

function(init_lib_binarytree target)
  target_sources(${target}
    PUBLIC
    "${BINARYTREE_DIR}/binarytree.h"
    PRIVATE
    "${BINARYTREE_DIR}/binarytree.c"
  )
endfunction()

add_library(trees STATIC)
init_lib_binarytree(trees)
