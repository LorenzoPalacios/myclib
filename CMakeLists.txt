cmake_minimum_required(VERSION 3.20)
project(myclib)

set(CMAKE_COMPILE_WARNING_AS_ERROR true)
set(CMAKE_C_STANDARD=17)
set(DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_compile_options(-O3 -Weverything -std=c89 -Wno-unsafe-buffer-usage -march=native)
add_library(myclib STATIC array/array.c random/random.c stack/stack.c str/str.c vector/vector.c)

target_include_directories(myclib PUBLIC
  "${PROJECT_SOURCE_DIR}/array"
  "${PROJECT_SOURCE_DIR}/lists"
  "${PROJECT_SOURCE_DIR}/queues"
  "${PROJECT_SOURCE_DIR}/random"
  "${PROJECT_SOURCE_DIR}/stack"
  "${PROJECT_SOURCE_DIR}/str"
  "${PROJECT_SOURCE_DIR}/trees"
  "${PROJECT_SOURCE_DIR}/vector")
