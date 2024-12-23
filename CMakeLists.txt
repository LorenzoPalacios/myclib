cmake_minimum_required(VERSION 3.20)
project(myclib)
add_compile_options(-std=c11 -O2 -Wall -Werror -Wextra -pedantic -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wstrict-overflow=4 -Wwrite-strings -Waggregate-return -Wcast-qual -Wconversion -Wformat=2 -mtune=generic)
add_executable(exe array/array.c random/random.c stack/stack.c stack/example.c strext/strext.c trees/binarytree/binarytree.c vector/vector.c)
