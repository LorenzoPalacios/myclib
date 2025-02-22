cmake_minimum_required(VERSION 3.11)
project(myclib)

set(CMAKE_COMPILE_WARNING_AS_ERROR true)
set(DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_compile_options(-O3 -pedantic -std=c89 -Wabi -Wall -Wcast-align -Wcast-qual -Wconversion -Werror -Wextra -Wformat=2 -Wshadow -Wstrict-overflow=5 -Wundef -Wunreachable-code -Wwrite-strings -march=native -mtune=native)
add_library(myclib STATIC array/array.c lists/singly/sll.c random/random.c stack/stack.c str/str.c vector/vector.c)

target_include_directories(myclib PUBLIC
  "${PROJECT_SOURCE_DIR}/array"
  "${PROJECT_SOURCE_DIR}/lists/singly"
  "${PROJECT_SOURCE_DIR}/random"
  "${PROJECT_SOURCE_DIR}/stack"
  "${PROJECT_SOURCE_DIR}/str"
  "${PROJECT_SOURCE_DIR}/trees"
  "${PROJECT_SOURCE_DIR}/vector")
