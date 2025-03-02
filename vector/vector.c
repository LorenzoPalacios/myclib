#include "vector.h"

#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/*
 *  `length`   - The maximum number of elements that can be held by the vector
 * until expansion is necessary.
 * `capacity`  - The number of elements the vector has allocated for.
 * `elem_size` - The size of each element in the vector.
 */
struct vector {
  size_t length;
  size_t capacity;
  size_t elem_size;
};

/* - INTERNAL -*/

static inline byte *internal_vector_get(struct vector *const vec,
                                        const size_t index) {
  return (byte *)(vec + 1) + (vec->elem_size * index);
}

/* - IMPLEMENTATION - */

bool vector_add_(struct vector **const vec, const void *const elem) {
  if ((*vec)->length == (*vec)->capacity)
    if (!vector_expand_(vec)) return false;
  void *dst = internal_vector_get(*vec, (*vec)->length);
  memcpy(dst, elem, (*vec)->elem_size);
  (*vec)->length++;
  return true;
}

void vector_clear_(struct vector *const vec) {
  memset(vec + 1, 0, vec->elem_size * vec->length);
}

vector vector_copy_(struct vector *const vec) {
  return vector_new_(vector_data_(vec), vec->elem_size, vec->length);
}

void *vector_data_(struct vector *const vec) { return vec + 1; }

void vector_delete_(struct vector **const vec) {
  free(*vec);
  *vec = NULL;
}

bool vector_expand_(struct vector **const vec) {
  const size_t CUR_LENGTH = (*vec)->length;
  const size_t IDEAL_LENGTH = (CUR_LENGTH == 0 ? 1 : 2 * CUR_LENGTH);
  /*
   * If the first (generous) expansion fails, retry with a second and final
   * conservative request, in which case `false` may still be returned if the
   * system is out of memory or otherwise unwilling to provide memory.
   */
  return vector_resize_(vec, IDEAL_LENGTH)
             ? true
             : vector_resize_(vec, CUR_LENGTH + 1);
}

void vector_for_each_(struct vector *const vec, const for_each_op operation) {
  const size_t LEN = vec->length;
  size_t index = 0;
  while (index < LEN && operation(internal_vector_get(vec, index))) index++;
}

void *vector_get_(struct vector *const vec, const size_t index) {
  if (index >= vec->length) return NULL;
  return internal_vector_get(vec, index);
}

vector vector_init_(const size_t elem_size, const size_t capacity) {
  vector vec = malloc((elem_size * capacity) + sizeof(struct vector));
  vec->length = 0;
  vec->capacity = capacity;
  vec->elem_size = elem_size;
  return vec;
}

bool vector_insert_(struct vector **const vec, const void *const elem,
                    const size_t index) {
  if ((*vec)->length == (*vec)->capacity)
    if (!vector_expand_(vec)) return false;
  if (internal_vector_get(*(vec), index) != elem) {
    const size_t ELEM_SIZE = (*vec)->elem_size;
    const size_t SHIFT_SIZE = ELEM_SIZE * ((*vec)->length - index);
    byte *const insertion_pos = internal_vector_get(*(vec), index);
    byte *const shift_pos = insertion_pos + ELEM_SIZE;
    memmove(shift_pos, insertion_pos, SHIFT_SIZE);
    memcpy(insertion_pos, elem, ELEM_SIZE);
    (*vec)->length++;
    return true;
  }
  return false;
}

size_t vector_length_(struct vector *const vec) { return vec->length; }

vector vector_new_(const void *const data, const size_t elem_size,
                   const size_t length) {
  vector vec = vector_init_(elem_size, length);
  memcpy(vector_data_(vec), data, vec->elem_size * vec->length);
  return vec;
}

bool vector_resize_(struct vector **const vec, const size_t new_length) {
  if (new_length != (*vec)->capacity) {
    const size_t ALLOCATION =
        (new_length * (*vec)->elem_size) + sizeof(struct vector);
    vector new_vec = realloc((*vec), ALLOCATION);
    if (new_vec == NULL) return false;
    *vec = new_vec;
    (*vec)->capacity = new_length;
    return true;
  }
  return false;
}

void vector_set_(struct vector *const vec, const void *const elem,
                 const size_t index) {
  if (index >= vec->length) return;
  if (elem != internal_vector_get(vec, index))
    memcpy(internal_vector_get(vec, index), elem, vec->elem_size);
}

void vector_set_range_(struct vector *const vec, const void *const elem,
                       const size_t start, const size_t end) {
  if (end <= vec->length) {
    const size_t ELEM_SIZE = vec->elem_size;
    void *const data = vector_data_(vec);
    for (size_t i = start; i < end; i++) memcpy(data, elem, ELEM_SIZE);
  }
}

bool vector_shrink_to_fit_(struct vector **const vec) {
  return vector_resize_(vec, (*vec)->length);
}
