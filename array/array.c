#include "array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

array *array_new_(const void *const data, const size_t elem_size,
                  const size_t length) {
  array *const new_arr = array_init(elem_size, length);
  if (new_arr == NULL) return NULL;
  memcpy(new_arr->data, data, elem_size * length);
  new_arr->length = length;
  return new_arr;
}

void *array_add(array *const arr, const void *const elem) {
  if (arr->length == arr->capacity) return NULL;
  void *const elem_pos = (byte *)arr->data + arr->length;
  memcpy(elem_pos, elem, arr->elem_size);
  arr->length++;
  return elem_pos;
}

void array_for_each(array *const arr, void (*const operation)(void *elem)) {
  const size_t LEN = arr->length;
  for (size_t i = 0; i < LEN; i++) operation(array_get(arr, i));
}

array *array_init(const size_t elem_size, const size_t length) {
  const size_t ELEM_ALLOC = elem_size * length;
  array *const new_arr = malloc(ELEM_ALLOC + sizeof(array));
  if (new_arr == NULL) return NULL;
  new_arr->data = (byte *)(new_arr + 1);
  new_arr->capacity = length;
  new_arr->length = 0;
  new_arr->elem_size = elem_size;
  return new_arr;
}

void array_delete(array *const arr) {
  free(arr);
}

void *array_get(const array *const arr, const size_t index) {
  if (index >= arr->length) return NULL;
  return (byte *)arr->data + (arr->elem_size * index);
}

void *array_insert(array *const arr, const void *const elem, size_t index) {
  if (arr->length == arr->capacity) return NULL;
  if (index > arr->length) return NULL;
  const size_t ELEM_SIZE = arr->elem_size;

  byte *const elem_pos = array_get(arr, index);
  byte *const after_elem_pos = elem_pos + ELEM_SIZE;
  const size_t REMAINDER_LENGTH = arr->length - index;
  memmove(after_elem_pos, elem_pos, ELEM_SIZE * REMAINDER_LENGTH);
  memcpy(elem_pos, elem, arr->elem_size);

  arr->length++;
  return elem_pos;
}

void array_clear(array *const arr) {
  const size_t ELEM_ALLOC = arr->elem_size * arr->capacity;
  memset(arr->data, 0, ELEM_ALLOC);
  arr->length = 0;
}
