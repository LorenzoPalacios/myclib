#include "array.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

array_t *_new_array(const void *const data, const size_t elem_size,
                    const size_t length) {
  array_t *const new_arr = init_array(elem_size, length);
  if (new_arr == NULL) return NULL;
  new_arr->length = length;
  memcpy(new_arr->data, data, elem_size * length);
  return new_arr;
}

void *add_elem(array_t *const arr, const void *const elem) {
  if (arr->length == arr->capacity) return NULL;
  void *const elem_pos = arr->data + arr->length;
  memcpy(elem_pos, elem, arr->elem_size);
  arr->length++;
  return elem_pos;
}

void for_each(array_t *const arr, void (*const op)(void *elem)) {
  const size_t LEN = arr->length;
  const size_t ELEM_SIZE = arr->elem_size;
  for (size_t i = 0; i < LEN; i++) op(arr->data + i * ELEM_SIZE);
}

array_t *init_array(const size_t elem_size, const size_t length) {
  const size_t ELEM_ALLOC = elem_size * length;
  array_t *const new_arr = malloc(ELEM_ALLOC + sizeof(array_t));
  if (new_arr == NULL) return NULL;
  new_arr->data = (byte *)(new_arr + 1);
  new_arr->capacity = length;
  new_arr->length = 0;
  new_arr->elem_size = elem_size;
  return new_arr;
}

void *insert_elem(array_t *const arr, const void *const elem, size_t index) {
  if (arr->length == arr->capacity) return NULL;
  if (index > arr->length) return NULL;
  const size_t ELEM_SIZE = arr->elem_size;
  byte *const elem_pos = arr->data + index * ELEM_SIZE;
  byte *const after_elem_pos = elem_pos + ELEM_SIZE;
  const size_t SUB_ARRAY_LENGTH = arr->length - index;
  memmove(after_elem_pos, elem_pos, ELEM_SIZE * SUB_ARRAY_LENGTH);
  memcpy(elem_pos, elem, arr->elem_size);
  arr->length++;
  return elem_pos;
}

void *get_elem(const array_t *const arr, const size_t index) {
  if (index >= arr->length) return NULL;
  return arr->data + arr->elem_size * index;
}

void _delete_array(array_t **const arr) {
  free(*arr);
  *arr = NULL;
}

void delete_array_s(array_t **const arr) {
  memset(*arr, 0, (*arr)->capacity + sizeof(array_t));
  _delete_array(arr);
}

void wipe_array(array_t *const arr) {
  const size_t ELEM_ALLOC = arr->elem_size * arr->capacity;
  memset(arr->data, 0, ELEM_ALLOC);
}
