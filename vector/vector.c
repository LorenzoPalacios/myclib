#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline size_t vector_get_data_alloc(const vector *const vec) {
  return vec->allocation - sizeof(vector);
}

inline size_t vector_get_capacity(const vector *const vec) {
  const size_t DATA_ALLOC = vector_get_data_alloc(vec);
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t CAPACITY = DATA_ALLOC / ELEM_SIZE;
  return CAPACITY;
}

static inline byte *get_data(vector *const vec) { return (byte *)(vec + 1); }

vector *_vector_new(const void *const data, const size_t elem_size,
                    const size_t length) {
  vector *vec = vector_init(elem_size, length);
  if (vec == NULL) return NULL;
  vec->length = length;
  const size_t DATA_ALLOC = vector_get_data_alloc(vec);
  byte *const VECTOR_DATA = get_data(vec);
  memcpy(VECTOR_DATA, data, DATA_ALLOC);
  return vec;
}

vector *vector_add_elem(vector *vec, const void *const elem) {
  {
    const size_t CAPACITY = vector_get_capacity(vec);
    if (vec->length == CAPACITY) {
      vec = vector_expand(vec);
      if (vec == NULL) return NULL;
    }
  }
  byte *const insertion_pos = get_data(vec) + vec->length * vec->elem_size;
  memcpy(insertion_pos, elem, vec->elem_size);
  vec->length++;
  return vec;
}

void vector_delete(vector **const vec) {
  free(*vec);
  *vec = NULL;
}

void vector_delete_s(vector **const vec) {
  memset(*vec, 0, vector_get_data_alloc(*vec));
  vector_delete(vec);
}

vector *vector_expand(vector *const vec) {
  const size_t CUR_CAPACITY = vector_get_capacity(vec);
  const size_t IDEAL_CAPACITY = (CUR_CAPACITY == 0) ? (1) : (2 * CUR_CAPACITY);
  vector *new_vec = vector_resize(vec, IDEAL_CAPACITY);
  if (new_vec == NULL) new_vec = vector_resize(vec, CUR_CAPACITY + 1);
  return new_vec;
}

void vector_for_each(vector *const vec, void (*const op)(void *elem)) {
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t LEN = vec->length;
  byte *const VECTOR_DATA = get_data(vec);
  for (size_t i = 0; i < LEN; i++) op(VECTOR_DATA + i * ELEM_SIZE);
}

vector *vector_init(const size_t elem_size, const size_t length) {
  const size_t ALLOCATION = length * elem_size + sizeof(vector);
  vector *const vec = malloc(ALLOCATION);
  if (vec == NULL) return NULL;
  vec->length = 0;
  vec->elem_size = elem_size;
  vec->allocation = ALLOCATION;
  return vec;
}

vector *vector_insert_elem(vector *vec, const void *const elem,
                           const size_t index) {
  if (index > vec->length) return vec;
  if (index == vec->length) return vector_add_elem(vec, elem);
  if (vec->length == vector_get_capacity(vec)) vec = vector_expand(vec);
  const size_t ELEM_SIZE = vec->elem_size;
  byte *const INSERTION_POS = get_data(vec) + index * ELEM_SIZE;
  byte *const SHIFT_POS = INSERTION_POS + ELEM_SIZE;
  const size_t SHIFT_SIZE = ELEM_SIZE * (vec->length - index);
  memmove(SHIFT_POS, INSERTION_POS, SHIFT_SIZE);
  memcpy(INSERTION_POS, elem, ELEM_SIZE);
  vec->length++;
  return vec;
}

vector *vector_resize(vector *vec, const size_t new_capacity) {
  putchar('a');
  {
    const size_t CUR_CAPACITY = vector_get_capacity(vec);
    if (CUR_CAPACITY == new_capacity) return vec;
  }
  putchar('b');
  const size_t NEW_SIZE = new_capacity * vec->elem_size + sizeof(vector);
  vec = realloc(vec, NEW_SIZE);
  if (vec == NULL) return NULL;
  vec->allocation = NEW_SIZE;
  if (vec->length > new_capacity) vec->length = new_capacity;
  return vec;
}

vector *vector_shrink_to_fit(vector *const vec) {
  return vector_resize(vec, vec->length);
}
