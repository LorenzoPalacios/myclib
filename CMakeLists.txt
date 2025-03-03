# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(myclib)

set(DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_compile_options(
  -O3 -pedantic -Wabi -Wall -Wcast-align -Wcast-qual -Wconversion -Werror
  -Wextra -Wformat=2 -Wshadow -Wstrict-overflow=5 -Wundef -Wunreachable-code
  -Wwrite-strings -march=native -mtune=native
  )

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR true)

add_subdirectory("lists")
add_subdirectory("random")
add_subdirectory("stack")
add_subdirectory("str")
add_subdirectory("tests")
#add_subdirectory("trees")
add_subdirectory("vector")
