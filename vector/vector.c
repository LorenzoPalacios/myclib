#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline size_t get_data_alloc(const vector *const vec) {
  return vec->capacity * vec->elem_size;
}

static inline size_t get_total_alloc(const vector *const vec) {
  return get_data_alloc(vec) + sizeof(vector);
}

vector *vector_add_elem(vector *vec, const void *const elem) {
  if (vec->length == vec->capacity) {
    vec = vector_expand(vec);
    if (vec == NULL) return NULL;
  }
  byte *const insertion_pos = vec->data + vec->length * vec->elem_size;
  memcpy(insertion_pos, elem, vec->elem_size);
  vec->length++;
  return vec;
}

void vector_delete(vector **const vec) {
  free(*vec);
  *vec = NULL;
}

void vector_delete_s(vector **vec) {
  memset(*vec, 0, (*vec)->capacity);
  vector_delete(vec);
}

void vector_for_each(vector *vec, void (*op)(void *elem)) {
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t LEN = vec->length;
  byte *const data = vec->data;
  for (size_t i = 0; i < LEN; i++)
    op(data + i * ELEM_SIZE);
}

vector *vector_init(const size_t elem_size, const size_t length) {
  const size_t CAPACITY = length * elem_size;
  vector *const vec = malloc(CAPACITY + sizeof(vector));
  if (vec == NULL) return NULL;
  vec->data = (byte *)(vec + 1);
  vec->length = 0;
  vec->capacity = length;
  vec->elem_size = elem_size;
  return vec;
}

static inline size_t sanitize_new_size(const vector *const vec,
                                       const size_t new_size) {
  if (new_size < sizeof(vector)) return sizeof(vector);
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t ALIGNED_SIZE = new_size - (new_size % ELEM_SIZE);
  return ALIGNED_SIZE;
}

vector *vector_resize(vector *vec, const size_t new_size) {
  const size_t SANITIZED_SIZE = sanitize_new_size(vec, new_size);
  vec = realloc(vec, SANITIZED_SIZE);
  if (vec == NULL) return NULL;
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t ELEM_ALLOC = SANITIZED_SIZE - sizeof(vector);
  const size_t CAPACITY = ELEM_ALLOC / ELEM_SIZE;
  vec->capacity = CAPACITY;
  if (vec->length > CAPACITY) vec->length = CAPACITY;
  vec->data = (byte*)(vec + 1);
  return vec;
}

vector *_vector_new(const void *const data, const size_t elem_size,
                    const size_t length) {
  vector *vec = vector_init(elem_size, length);
  if (vec == NULL) return NULL;
  vec->length = length;
  const size_t ELEM_ALLOC = length * elem_size;
  memcpy(vec->data, data, ELEM_ALLOC);
  return vec;
}
