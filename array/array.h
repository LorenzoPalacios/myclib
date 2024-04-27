#ifndef ARRAY_H
#define ARRAY_H

#include <string.h>

#include "../MyBasics.h"

typedef struct array {
  void *data;
  size_t num_elems;
  size_t allocated_size;
  size_t indexing_offset;
} array;

array *create_array(const void *const data, const size_t elem_size,
                    const size_t num_elems);

void *get_elem(const array *const arr, const size_t index);

void delete_array(array *arr);

void zero_and_delete_array(array *arr);

#endif