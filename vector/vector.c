#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline size_t get_data_alloc(const vector *const vec) {
  return vec->capacity * vec->elem_size;
}

static inline byte *get_data(vector *const vec) { return (byte *)(vec + 1); }

vector *vector_add_elem(vector *vec, const void *const elem) {
  if (vec->length == vec->capacity) {
    vec = vector_expand(vec);
    if (vec == NULL) return NULL;
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

void vector_delete_s(vector **vec) {
  memset(*vec, 0, (*vec)->capacity);
  vector_delete(vec);
}

void vector_for_each(vector *const vec, void (*const op)(void *elem)) {
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t LEN = vec->length;
  byte *const VECTOR_DATA = get_data(vec);
  for (size_t i = 0; i < LEN; i++) op(VECTOR_DATA + i * ELEM_SIZE);
}

vector *vector_init(const size_t elem_size, const size_t length) {
  const size_t CAPACITY = length * elem_size;
  vector *const vec = malloc(CAPACITY + sizeof(vector));
  if (vec == NULL) return NULL;
  vec->length = 0;
  vec->capacity = length;
  vec->elem_size = elem_size;
  return vec;
}

vector *vector_resize(vector *vec, const size_t new_length) {
  const size_t ELEM_SIZE = vec->elem_size;
  const size_t NEW_SIZE = ELEM_SIZE + sizeof(vector);
  vec = realloc(vec, NEW_SIZE);
  if (vec == NULL) return NULL;
  vec->capacity = new_length;
  if (vec->length > new_length) vec->length = new_length;
  return vec;
}

vector *_vector_new(const void *const data, const size_t elem_size,
                    const size_t length) {
  vector *vec = vector_init(elem_size, length);
  if (vec == NULL) return NULL;
  vec->length = length;
  const size_t ELEM_ALLOC = length * elem_size;
  byte *const VECTOR_DATA = get_data(vec);
  memcpy(VECTOR_DATA, data, ELEM_ALLOC);
  return vec;
}

static void print(void *elem) { printf("%d ", *(int *)elem); }

int main(void) {
  vector *v;
  {
    const int data[] = {1, 2, 3, 4, 5, 6};
    v = vector_new(data);
  }
  vector_for_each(v, print);
}
