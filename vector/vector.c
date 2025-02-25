#include "vector.h"

#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static inline byte *internal_vector_get(const vector *const vec,
                                        const size_t index) {
  return (byte *)vec->data + (vec->elem_size * index);
}

void vector_clear_(const vector *const vec) {
  memset(vec->data, 0, vec->elem_size * vec->length);
}

void vector_delete_(const vector *const vec) { free(vec->data); }

bool vector_expand_(vector *const vec) {
  const size_t CUR_LENGTH = vec->capacity;
  const size_t IDEAL_LENGTH = (CUR_LENGTH == 0 ? 1 : 2 * CUR_LENGTH);
  const bool EXPANSION_SUCCESS = vector_resize_(vec, IDEAL_LENGTH);
  if (!EXPANSION_SUCCESS) return vector_resize_(vec, CUR_LENGTH + 1);
  return EXPANSION_SUCCESS;
}

void vector_for_each_(vector *const vec, const for_each_op operation) {
  const size_t LEN = vec->length;
  size_t index = 0;
  while (index < LEN && operation(internal_vector_get(vec, index))) index++;
}

void *vector_get_(const vector *const vec, const size_t index) {
  if (index >= vec->length) return NULL;
  return internal_vector_get(vec, index);
}

vector vector_init_(const size_t elem_size, const size_t length) {
  vector vec;
  vec.data = malloc(elem_size * length);
  vec.length = vec.capacity = length;
  vec.elem_size = elem_size;
  return vec;
}

void vector_insert_(vector *const vec, const void *const elem,
                    const size_t index) {
  if (index >= vec->length) vector_set_(vec, elem, index);
  if (internal_vector_get(vec, index) != elem) {
    byte *const insertion_pos = internal_vector_get(vec, index);
    const size_t ELEM_SIZE = vec->elem_size;
    const size_t SHIFT_SIZE = ELEM_SIZE * (vec->length - index - 1);
    byte *const shift_pos = insertion_pos + ELEM_SIZE;
    memmove(shift_pos, insertion_pos, SHIFT_SIZE);
    memcpy(insertion_pos, elem, ELEM_SIZE);
  }
}

vector vector_new_(const void *const data, const size_t elem_size,
                   const size_t length) {
  const vector vec = vector_init_(elem_size, length);
  memcpy(vec.data, data, vec.elem_size * vec.length);
  return vec;
}

bool vector_resize_(vector *const vec, const size_t new_length) {
  if (vec->length == new_length) return false;
  if (new_length < vec->capacity || new_length > vec->capacity) {
    const size_t ALLOCATION = new_length * vec->elem_size;
    byte *const new_data = realloc(vec->data, ALLOCATION);
    if (new_data == NULL) return false;
    vec->data = new_data;
    vec->capacity = new_length;
  }
  vec->length = new_length;
  return true;
}

void vector_set_(vector *const vec, const void *const elem,
                 const size_t index) {
  if (index >= vec->length) vector_resize_(vec, index + 1);
  if (elem != internal_vector_get(vec, index))
    memcpy(internal_vector_get(vec, index), elem, vec->elem_size);
}

bool vector_shrink_to_fit_(vector *const vec) {
  return vector_resize_(vec, vec->length);
}
