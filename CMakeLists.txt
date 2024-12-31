cmake_minimum_required(VERSION 3.20)
project(myclib)

set(CMAKE_COMPILE_WARNING_AS_ERROR true)
set(CMAKE_C_STANDARD=11)

add_compile_options(-O2 -Wall -Werror -Wextra -pedantic -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wstrict-overflow=4 -Wwrite-strings -Waggregate-return -Wcast-qual -Wconversion -Wformat=2 -march=native -fsanitize=safe-stack)
add_library(myclib STATIC array/array.c random/random.c stack/stack.c strext/strext.c vector/vector.c)

target_include_directories(myclib PUBLIC
  "${PROJECT_SOURCE_DIR}/array"
  "${PROJECT_SOURCE_DIR}/lists"
  "${PROJECT_SOURCE_DIR}/queues"
  "${PROJECT_SOURCE_DIR}/random"
  "${PROJECT_SOURCE_DIR}/stack" 
  "${PROJECT_SOURCE_DIR}/strext"
  "${PROJECT_SOURCE_DIR}/trees"
  "${PROJECT_SOURCE_DIR}/vector")
