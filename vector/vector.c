#include "vector.h"

#include <stdbool.h>
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

byte *vector_contents(const vector *const vec) { return vec->data; }

void vector_delete_(const vector *const vec) { free(vec->data); }

bool vector_expand_(vector *const vec) {
  const size_t CUR_LENGTH = vec->capacity;
  const size_t IDEAL_LENGTH = (CUR_LENGTH == 0 ? 1 : 2 * CUR_LENGTH);
  const bool EXPANSION_SUCCESS = vector_resize_(vec, IDEAL_LENGTH);
  if (!EXPANSION_SUCCESS) return vector_resize_(vec, CUR_LENGTH + 1);
  return EXPANSION_SUCCESS;
}

void vector_for_each_(vector *const vec, bool (*const operation)(void *elem)) {
  const size_t LEN = vec->length;
  const size_t ELEM_SIZE = vec->elem_size;
  byte *const contents = vec->data;
  for (size_t i = 0; i < LEN && operation(contents + (i * ELEM_SIZE)); i++);
}

void *vector_get_(const vector *const vec, const size_t index) {
  if (index >= vec->length) return NULL;
  return internal_vector_get(vec, index);
}

vector vector_init_(const size_t elem_size, const size_t length) {
  return (vector){.data = malloc(elem_size * length),
                  .length = length,
                  .capacity = length,
                  .elem_size = elem_size};
}

void vector_insert_(vector *const vec, const void *const elem,
                    const size_t index) {
  if (index > vec->length) return;
  if (index == vec->length) {
    vector_expand_(vec);
    vector_set_(vec, elem, index);
  }
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t SHIFT_SIZE = ELEM_SIZE * (vec->length - index);
  byte *const insertion_pos = internal_vector_get(vec, index);
  byte *const shift_pos = insertion_pos + ELEM_SIZE;
  memmove(shift_pos, insertion_pos, SHIFT_SIZE);
  memcpy(insertion_pos, elem, ELEM_SIZE);
}

vector vector_new_(const void *const data, const size_t elem_size,
                   const size_t length) {
  const vector vec = vector_init(elem_size, length);
  memcpy(vec.data, data, vec.elem_size * vec.length);
  return vec;
}

bool vector_resize_(vector *const vec, const size_t new_length) {
  if (vec->length == new_length) return false;
  if (new_length > vec->capacity) {
    const size_t ALLOCATION = new_length * vec->elem_size;
    byte *const new_data = realloc(vec->data, ALLOCATION);
    if (new_data == NULL) return false;
    vec->data = new_data;
    vec->capacity = new_length;
  }
  vec->length = new_length;
  return true;
}

void vector_set_(const vector *const vec, const void *const elem,
                 const size_t index) {
  if (index >= vec->length) return;
  byte *const target = internal_vector_get(vec, index);
  if (elem != target) memcpy(target, elem, vec->elem_size);
}

bool vector_shrink_to_fit_(vector *const vec) {
  return vector_resize_(vec, vec->length);
}
