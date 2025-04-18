#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/compat.h"
#include "../include/utilmyclib.h"

/* - DEFINITIONS - */

#define vector(type) type *

/* - INTERNAL USE ONLY - */

#define VEC_EXPANSION_FACTOR (2)

/*
 * `capacity` - The total number of elements a vector has allocated for.
 *  `length`  - The number of elements currently held by a vector.
 */
typedef struct {
  size_t capacity;
  size_t length;
} vec_header;

#define vector_header(vec) ((vec_header *)((byte *)(vec) - sizeof(vec_header)))

#define vector_header_const(vec) \
  ((const vec_header *)((const byte *)(vec) - sizeof(vec_header)))

/* - CONVENIENCE MACROS - */

/*
 * All "_s" variants of the below macros do not evaluate their arguments more
 * than once.
 */

/* Unary `+` for rvalue conversion, preventing assignment. */
#define vector_capacity(vec) (+vector_header_const(vec)->capacity)

#define vector_clear(vec)                                     \
  ((void)(memset(vec, 0, sizeof *(vec) * vector_length(vec)), \
          vector_header(vec)->length = 0))

#define vector_clear_s(vec) vector_untyped_clear(vec, sizeof *(vec))

#define vector_copy(vec)                                                       \
  ((void *)((vec_header *)memcpy(                                              \
                malloc((sizeof *(vec) * vector_capacity(vec)) +                \
                       sizeof(vec_header)),                                    \
                vector_header_const(vec),                                      \
                (sizeof *(vec) * vector_capacity(vec)) + sizeof(vec_header)) + \
            1))

#define vector_copy_s(vec) vector_untyped_copy(vec, sizeof *(vec))

#define vector_delete(vec) ((void)(free(vector_header(vec)), (vec) = NULL))

#define vector_delete_s(vec) vector_untyped_delete((void **)&(vec))

#define vector_expand(vec)                                            \
  (vector_resize(vec, vector_length(vec) != 0                         \
                          ? VEC_EXPANSION_FACTOR * vector_length(vec) \
                          : 1))

#define vector_expand_s(vec) \
  vector_untyped_expand((void **)&(vec), sizeof *(vec))

#define vector_get(vec, index) \
  (util_assert((size_t)(index) < vector_length(vec)), (vec)[(size_t)(index)])

#define vector_get_s(vec, index) vector_untyped_get(vec, index, sizeof *(vec))

#define vector_is_empty(vec) (vector_length(vec) == 0)

/* Unary `+` for rvalue conversion, preventing assignment. */
#define vector_length(vec) (+vector_header_const(vec)->length)

#define vector_new(type, capacity) \
  ((type *)vector_untyped_new(sizeof(type), capacity))

#define vector_push(vec, elem)                       \
  ((void)vector_resize(vec, vector_length(vec) + 1), \
   (vec)[vector_length(vec) - 1] = (elem))

#define vector_push_s(vec, elem) \
  vector_untyped_push((void **)&(vec), (const void *)&(elem), sizeof *(vec))

/* clang-format off */

#define vector_resize(vec, new_length)                                        \
(inline_if                                                                    \
(                                                                             \
  vector_capacity(vec) >= (size_t)(new_length),                               \
  (                                                                           \
    inline_if(                                                                \
      (size_t)(new_length) > vector_length(vec),                              \
      memset(                                                                 \
        (void *)((vec) + vector_length(vec)),                                 \
        0,                                                                    \
        sizeof*(vec) * ((new_length) - vector_length(vec))                    \
      ),                                                                      \
      NULL                                                                    \
    ),                                                                        \
    vector_header(vec)->length = (new_length)                                 \
  ),                                                                          \
  (                                                                           \
    (vec) = (void *)(((vec_header *)realloc(                                  \
      vector_header(vec), (sizeof *(vec) * (new_length)) + sizeof(vec_header) \
      )) + 1                                                                  \
    ),                                                                        \
    vector_header(vec)->length = vector_header(vec)->capacity = (new_length)  \
  )                                                                           \
), (vec))

/* clang-format on */

#define vector_resize_s(vec, new_length) \
  vector_untyped_resize((void **)&(vec), new_length, sizeof *(vec))

#define vector_set(vec, elem, index) \
  (util_assert((index) < vector_length(vec)), ((vec)[index] = (elem)))

#define vector_set_s(vec, elem, index) \
  vector_untyped_set((void **)&(vec), &(elem), index, sizeof *(vec))

#define vector_shrink(vec)                                            \
  ((void)((vec) = (void *)((vec_header *)(realloc(                    \
                               vector_header(vec),                    \
                               (sizeof *(vec) * vector_length(vec)) + \
                                   sizeof(vec_header))) +             \
                           1)),                                       \
   (void)(vector_header(vec)->capacity = vector_length(vec)), (vec))

#define vector_shrink_s(vec) \
  vector_untyped_shrink((void **)&(vec), sizeof *(vec))

/* - FUNCTION DECLARATIONS - */

static void vector_untyped_clear(void *vec, size_t elem_size);
static void *vector_untyped_copy(const void *vec, size_t elem_size);
static void vector_untyped_delete(void **vec);
static void *vector_untyped_expand(void **vec, size_t elem_size);
static void *vector_untyped_get(void *vec, size_t index, size_t elem_size);
static void *vector_untyped_new(size_t elem_size, size_t num_elems);
static void *vector_untyped_push(void **vec, const void *elem,
                                 size_t elem_size);
static void *vector_untyped_resize(void **vec, size_t new_length,
                                   size_t elem_size);
static void *vector_untyped_set(void **vec, const void *elem, size_t index,
                                size_t elem_size);
static void *vector_untyped_shrink(void **vec, size_t elem_size);

/* - FUNCTION DEFINITIONS - */

static inline void vector_untyped_clear(void *const vec,
                                        const size_t elem_size) {
  memset(vec, 0, elem_size * vector_length(vec));
  vector_header(vec)->length = 0;
}

static inline void *vector_untyped_copy(const void *const vec,
                                        const size_t elem_size) {
  const size_t ALLOCATION =
      (elem_size * vector_capacity(vec)) + sizeof(vec_header);
  void *vec_copy = malloc(ALLOCATION);
  if (vec_copy != NULL) {
    memcpy(vec_copy, vector_header_const(vec), ALLOCATION);
    vec_copy = (vec_header *)vec_copy + 1;
  }
  return vec_copy;
}

static inline void vector_untyped_delete(void **const vec) {
  free(vector_header(*vec));
  *vec = NULL;
}

static inline void *vector_untyped_expand(void **const vec,
                                          const size_t elem_size) {
  const size_t LENGTH = vector_length(*vec);
  void *expansion_attempt =
      vector_untyped_resize(vec, VEC_EXPANSION_FACTOR * LENGTH, elem_size);
  if (expansion_attempt == NULL) {
    expansion_attempt = vector_untyped_resize(vec, LENGTH + 1, elem_size);
    if (expansion_attempt == NULL) return NULL;
  }
  *vec = expansion_attempt;
  return expansion_attempt;
}

static inline void *vector_untyped_get(void *const vec, const size_t index,
                                       const size_t elem_size) {
  util_assert(index < vector_length(vec));
  return (byte *)vec + (elem_size * index);
}

static inline void *vector_untyped_new(const size_t elem_size,
                                       const size_t num_elems) {
  vec_header *const vec = malloc((elem_size * num_elems) + sizeof(vec_header));
  vec->capacity = num_elems;
  vec->length = 0;
  return vec + 1;
}

static inline void *vector_untyped_push(void **const vec,
                                        const void *const elem,
                                        const size_t elem_size) {
  return vector_untyped_set(vec, elem, vector_length(*vec), elem_size);
}

static inline void *vector_untyped_resize(void **const vec,
                                          const size_t new_length,
                                          const size_t elem_size) {
  register vec_header *header = vector_header(*vec);
  if (new_length <= header->capacity) {
    header->length = new_length;
  } else {
    const size_t ALLOCATION = (elem_size * new_length) + sizeof(vec_header);
    header = realloc(header, ALLOCATION);
    if (header == NULL) return NULL;
    header->capacity = new_length;
    header->length = new_length;
    *vec = header + 1;
  }
  return header + 1;
}

static inline void *vector_untyped_set(void **const vec, const void *const elem,
                                       const size_t index,
                                       const size_t elem_size) {
  void *vec_actual = *vec;
  const size_t LENGTH = vector_length(vec_actual);
  if (index >= LENGTH) {
    if (vector_untyped_resize(&vec_actual, index + 1, elem_size) != NULL) {
      void *const clearance_start = (byte *)vec_actual + (elem_size * LENGTH);
      const size_t CLEARANCE_EXTENT = elem_size * (index - LENGTH);
      memset(clearance_start, 0, CLEARANCE_EXTENT);
      *vec = vec_actual;
    } else {
      return NULL;
    }
  }
  {
    void *const dst = vector_untyped_get(vec_actual, index, elem_size);
    memcpy(dst, elem, elem_size);
    return dst;
  }
}

static inline void *vector_untyped_shrink(void **const vec,
                                          const size_t elem_size) {
  const size_t ALLOCATION = elem_size * vector_length(*vec) + sizeof(vec_header);
  vec_header *shrunk_vec = realloc(vector_header(*vec), ALLOCATION);
  if (shrunk_vec != NULL) {
    shrunk_vec->capacity = shrunk_vec->length;
    *vec = shrunk_vec + 1;
  }
  return shrunk_vec;
}

#endif
