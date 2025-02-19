#include "array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static inline void *internal_array_get(const array *const arr,
                                       const size_t index) {
  return (byte *)arr->data + (index * arr->elem_size);
}

array array_new_(const void *const data, const size_t elem_size,
                 const size_t length) {
  array arr = array_init_(elem_size, length);
  memcpy(arr.data, data, elem_size * length);
  return arr;
}

void array_set_(const array *const arr, const void *const elem,
                const size_t index) {
  if (index < arr->length) {
    void *const elem_pos = internal_array_get(arr, index);
    memcpy(elem_pos, elem, arr->elem_size);
  }
}

void array_for_each_(array *const arr, const for_each_op operation) {
  const size_t LEN = arr->length;
  size_t index = 0;
  while (index < LEN && operation(internal_array_get(arr, index))) index++;
}

array array_init_(const size_t elem_size, const size_t length) {
  array arr;
  arr.data = malloc(elem_size * length);
  arr.length = length;
  arr.elem_size = elem_size;
  return arr;
}

void array_delete_(const array *const arr) { free(arr->data); }

void *array_get_(const array *const arr, const size_t index) {
  if (index >= arr->length) return NULL;
  return (byte *)arr->data + (arr->elem_size * index);
}

void array_insert_(const array *const arr, const void *const elem,
                   size_t index) {
  if (index < arr->length) {
    const size_t ELEM_SIZE = arr->elem_size;
    byte *const elem_pos = array_get_(arr, index);
    byte *const after_elem_pos = elem_pos + ELEM_SIZE;
    const size_t REMAINDER_LENGTH = arr->length - index;
    memmove(after_elem_pos, elem_pos, ELEM_SIZE * REMAINDER_LENGTH);
    memcpy(elem_pos, elem, arr->elem_size);
  }
}

void array_clear_(const array *const arr) {
  const size_t ELEM_ALLOC = arr->elem_size * arr->length;
  memset(arr->data, 0, ELEM_ALLOC);
}
