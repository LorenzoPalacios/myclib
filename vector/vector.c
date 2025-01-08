#include "vector.h"

#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

vector *vector_append(vector *vec, const void *const elem) {
  {
    const size_t CAPACITY = vector_capacity(vec);
    if (vec->length == CAPACITY) {
      vec = vector_expand(vec);
      if (vec == NULL) return NULL;
    }
  }
  byte *const insertion_pos = vector_get(vec, vec->length);
  memcpy(insertion_pos, elem, vec->elem_size);
  vec->length++;
  return vec;
}

size_t vector_capacity(const vector *const vec) {
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t DATA_ALLOC = vec->allocation - sizeof(vector);
  const size_t CAPACITY = DATA_ALLOC / ELEM_SIZE;
  return CAPACITY;
}

void *vector_data(vector *const vec) { return vec + 1; }

void vector_delete_(vector **const vec) {
  free(*vec);
  *vec = NULL;
}

void vector_delete_s_(vector **const vec) {
  memset(*vec, 0, (*vec)->allocation);
  vector_delete_(vec);
}

vector *vector_expand(vector *const vec) {
  const size_t CUR_CAPACITY = vector_capacity(vec);
  const size_t IDEAL_CAPACITY = (CUR_CAPACITY == 0) ? (1) : (2 * CUR_CAPACITY);
  vector *new_vec = vector_resize(vec, IDEAL_CAPACITY);
  if (new_vec == NULL) new_vec = vector_resize(vec, CUR_CAPACITY + 1);
  return new_vec;
}

void vector_for_each(vector *const vec, void (*const operation)(void *elem)) {
  const size_t LEN = vec->length;
  for (size_t i = 0; i < LEN; i++) operation(vector_get(vec, i));
}

void *vector_get(vector *const vec, const size_t index) {
  if (index >= vec->length) return NULL;
  return (byte *)vector_data(vec) + (index * vec->elem_size);
}

vector *vector_init(const size_t elem_size, const size_t length) {
  const size_t ALLOCATION = (length * elem_size) + sizeof(vector);
  vector *const vec = malloc(ALLOCATION);
  if (vec == NULL) return NULL;
  vec->length = 0;
  vec->elem_size = elem_size;
  vec->allocation = ALLOCATION;
  return vec;
}

vector *vector_insert(vector *vec, const void *const elem, const size_t index) {
  if (index > vec->length) return vec;
  if (vec->length == vector_capacity(vec)) vec = vector_expand(vec);

  const size_t ELEM_SIZE = vec->elem_size;
  const size_t SHIFT_SIZE = ELEM_SIZE * (vec->length - index);
  byte *const insertion_pos = vector_get(vec, index);
  byte *const shift_pos = insertion_pos + ELEM_SIZE;
  memmove(shift_pos, insertion_pos, SHIFT_SIZE);
  memcpy(insertion_pos, elem, ELEM_SIZE);

  vec->length++;
  return vec;
}

vector *vector_new_(const void *const data, const size_t elem_size,
                    const size_t length) {
  vector *const vec = vector_init(elem_size, length);
  if (vec == NULL) return NULL;
  vec->length = length;
  const size_t DATA_ALLOC = elem_size * length;
  memcpy(vector_data(vec), data, DATA_ALLOC);
  return vec;
}

vector *vector_resize(vector *vec, const size_t new_capacity) {
  {
    const size_t CUR_CAPACITY = vector_capacity(vec);
    if (CUR_CAPACITY == new_capacity) return vec;
  }

  const size_t ALLOCATION = (new_capacity * vec->elem_size) + sizeof(vector);
  vector *const new_vec = realloc(vec, ALLOCATION);
  if (new_vec == NULL) return NULL;
  vec = new_vec;
  vec->allocation = ALLOCATION;
  if (vec->length > new_capacity) vec->length = new_capacity;

  return vec;
}

vector *vector_shrink_to_fit(vector *const vec) {
  return vector_resize(vec, vec->length);
}
