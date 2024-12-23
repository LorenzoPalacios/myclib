cmake_minimum_required(VERSION 3.20)
project(myclib)
add_compile_options(-O2 -Wall -Werror -Wextra -pedantic -std=c11)
add_executable(exe array/array.c random/random.c strext/strext.c trees/binarytree/binarytree.c vector/vector.c)
