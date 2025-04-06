# This version is subject to change as we figure out what standards we will use.
cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

project(myclib)

set(BUILD_TESTS ON)

set(DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_compile_options(
  -D_FORTIFY_SOURCE=1 -march=native -mtune=native -O3 -pedantic -Wall
  -Wcast-align -Wcast-qual -Wconversion -Werror -Wextra -Wformat=2 -Wshadow
  -Wstrict-overflow=5 -Wundef -Wunreachable-code -Wwrite-strings
)

set(CMAKE_C_STANDARD 90) # This can be freely adjusted.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR true)

add_subdirectory("random")
add_subdirectory("stack")
add_subdirectory("str")
# add_subdirectory("trees")
add_subdirectory("vector")

target_include_directories(random PUBLIC "include")
target_include_directories(stack PUBLIC "include")
target_include_directories(str PUBLIC "include")
# target_include_directories(trees PUBLIC "include")
target_include_directories(vector PUBLIC "include")

if (BUILD_TESTS)
add_subdirectory("tests")
add_dependencies(tests stack)
add_dependencies(tests vector)
endif()
