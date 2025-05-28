#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"

/* - DEFINITIONS - */

#define vector(type) type *

#define VEC_BAD_INDEX ((size_t)-1)

/*
 * The single parameter is an array of pointers whose contents are as follows:
 *
 * The first element is the vector itself.
 * The second element is the current element of the vector in the for-each
 * sequence.
 * The third element is a pointer to any additional arguments that were
 * specified, except for when its value is `NULL`, in which case this element
 * will not exist.
 */
typedef void (*vec_for_each_op)(void *args[]);

typedef void (*vec_for_each_op_const)(const void *args[]);

/* - INTERNAL USE ONLY - */

#define VEC_EXPANSION_FACTOR (2)

/*
 * `capacity` - The total number of elements a vector has allocated for.
 *  `length`  - The number of elements currently held by a vector.
 */
typedef struct {
  size_t capacity;
  size_t length;
} vector_header;

#define vector_header(vec) ((vector_header *)(vec) - 1)

#define vector_header_const(vec) ((const vector_header *)(vec) - 1)

/* - CONVENIENCE MACROS - */

/*
 * All "_s" variants of the below macros do not evaluate their arguments more
 * than once.
 */

#define vector_capacity(vec) (+vector_header_const(vec)->capacity)

#define vector_clear(vec) \
  ((void)(memset(vec, 0, sizeof *(vec) * vector_length(vec))))

#define vector_clear_s(vec) vector_untyped_clear(vec, sizeof *(vec))

#define vector_copy(vec)                                                       \
  ((void *)((vector_header *)memcpy(                                              \
                malloc((sizeof *(vec) * vector_capacity(vec)) +                \
                       sizeof(vector_header)),                                    \
                vector_header_const(vec),                                      \
                (sizeof *(vec) * vector_capacity(vec)) + sizeof(vector_header)) + \
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

/*
 * C99 and later versions of C support variable arguments to macros, which can
 * be helpful if a user passes an `expr` containing commas, such as in the
 * following example:
 *
 * `vector_for_each(int n, vec, n++, n--);`
 *
 * where `n++, n--` is the intended `expr`.
 *
 * In contrast, prior versions of C (e.g. C90) require that such an `expr` be
 * enclosed in parentheses, like so:
 *
 * `vector_for_each(int n, vec, (n++, n--));`
 */
#if (IS_STDC99)
/*
 * The identifer for the iterator is `i`. The iterator exists in a separate
 * scope from all other declarations and may be accessed from `expr`.
 *
 * A terminating semicolon for `expr` is optional.
 */
#define vector_for_each(var_decl, vec, ...)    \
  {                                            \
    size_t i;                                  \
    for (i = 0; i < vector_length(vec); i++) { \
      var_decl = (vec)[i];                     \
      {                                        \
        __VA_ARGS__;                           \
      }                                        \
    }                                          \
  }                                            \
  (void)0
#else
#define vector_for_each(var_decl, vec, expr)   \
  {                                            \
    size_t i;                                  \
    for (i = 0; i < vector_length(vec); i++) { \
      var_decl = (vec)[i];                     \
      {                                        \
        expr;                                  \
      }                                        \
    }                                          \
  }                                            \
  (void)0
#endif

#define vector_for_each_c_s(vec, op, args) \
  vector_untyped_for_each_c(vec, op, args, sizeof *(vec))

#define vector_for_each_s(vec, op, args) \
  vector_untyped_for_each(vec, op, args, sizeof *(vec))

#define vector_get(vec, index) \
  ((vec)[util_assert((size_t)(index) < vector_length(vec)), (size_t)(index)])

#define vector_get_s(vec, index) vector_untyped_get(vec, index, sizeof *(vec))

#define vector_index_of(vec, elem) \
  vector_untyped_index_of(vec, &(elem), sizeof *(vec))

#define vector_insert(vec, elem, index)                                     \
  (inline_if((index) < vector_length(vec),                                  \
             ((void)(vector_resize(vec, vector_length(vec) + 1)),           \
              memmove((vec) + (index) + 1, (vec) + (index),                 \
                      sizeof *(vec) * (vector_length(vec) - (index) - 1))), \
             vector_resize(vec, (index) + 1)),                              \
   (vec)[index] = (elem))

#define vector_insert_s(vec, elem, index) \
  vector_untyped_insert((void **)&(vec), &(elem), index, sizeof *(vec))

#define vector_is_empty(vec) (vector_length(vec) == 0)

#define vector_length(vec) (+vector_header_const(vec)->length)

#define vector_new(type, capacity) \
  ((type *)vector_untyped_new(sizeof(type), capacity))

#define vector_pop(vec)                                                        \
  (inline_if(!vector_is_empty(vec), (void)vector_header(vec)->length--, NULL), \
   (vec)[vector_length(vec)])

#define vector_pop_s(vec) vector_untyped_pop(vec, sizeof *(vec))

#define vector_push(vec, elem)                       \
  ((void)vector_resize(vec, vector_length(vec) + 1), \
   (vec)[vector_length(vec) - 1] = (elem))

#define vector_push_s(vec, elem) \
  vector_untyped_push((void **)&(vec), (const void *)&(elem), sizeof *(vec))

#define vector_remove(vec, index)                                      \
  (util_assert((size_t)(index) < vector_length(vec)),                  \
   (void)(memmove((void *)((vec) + (index)),                           \
                  (const void *)((vec) + (index) + 1),                 \
                  sizeof *(vec) * (vector_length(vec) - (index) - 1)), \
          vector_header(vec)->length--))

#define vector_remove_s(vec, index) \
  vector_untyped_remove(vec, index, sizeof *(vec))

#define vector_reset(vec) ((void)(vector_header(vec)->length = 0))

/* clang-format off */

#define vector_resize(vec, new_length)                                        \
(                                                                             \
  inline_if                                                                   \
    ( /* Condition (depth 1) - Is the vector expanding? */                    \
      vector_length(vec) < (new_length),                                      \
      ( /* True branch (depth 1) */                                           \
        inline_if                                                             \
          ( /* Condition (depth 2) - Is there enough capacity? */             \
            vector_capacity(vec) < (new_length),                              \
            ( /* True branch (depth 2) - Add enough capacity. */              \
              (vec) = (void *)                                                \
                (                                                             \
                  1 + (vector_header *)realloc                                   \
                  (                                                           \
                    vector_header(vec),                                       \
                    sizeof(vector_header) + (sizeof *(vec) * (new_length))       \
                  )                                                           \
                ),                                                            \
              util_assert((vec) != NULL),                                     \
              vector_header(vec)->capacity = (new_length)                     \
            ),                                                                \
            ( /* False branch (depth 2) */                                    \
              NULL                                                            \
            )                                                                 \
          ),                                                                  \
          memset((void *)((vec) + vector_length(vec)), 0,                     \
                  sizeof*(vec) * ((new_length) - vector_length(vec)))         \
      ),                                                                      \
      ( /* False branch (depth 1) */                                          \
        NULL                                                                  \
      )                                                                       \
    ), (void)(vector_header(vec)->length = (new_length)), (vec)               \
)

/* clang-format on */
#define vector_resize_s(vec, new_length) \
  vector_untyped_resize((void **)&(vec), new_length, sizeof *(vec))

#define vector_set(vec, elem, index) \
  (util_assert((index) < vector_length(vec)), ((vec)[index] = (elem)))

#define vector_set_s(vec, elem, index) \
  vector_untyped_set((void **)&(vec), &(elem), index, sizeof *(vec))

#define vector_shrink(vec)                                            \
  ((void)((vec) = (void *)((vector_header *)(realloc(                    \
                               vector_header(vec),                    \
                               (sizeof *(vec) * vector_length(vec)) + \
                                   sizeof(vector_header))) +             \
                           1)),                                       \
   (void)(vector_header(vec)->capacity = vector_length(vec)), (vec))

#define vector_shrink_s(vec) \
  vector_untyped_shrink((void **)&(vec), sizeof *(vec))

/* - FUNCTION DECLARATIONS - */

static void vector_untyped_clear(vector(void) vec, size_t elem_size);
static void *vector_untyped_copy(const vector(void) vec, size_t elem_size);
static void vector_untyped_delete(vector(void) * vec);
static void *vector_untyped_expand(vector(void) * vec, size_t elem_size);
static void vector_untyped_for_each(vector(void) vec, vec_for_each_op op,
                                    void *args, size_t elem_size);
static void vector_untyped_for_each_c(const vector(void) vec,
                                      vec_for_each_op_const op,
                                      const void *args, size_t elem_size);
static void *vector_untyped_get(vector(void) vec, size_t index,
                                size_t elem_size);
static size_t vector_untyped_index_of(const vector(void) vec, const void *elem,
                                      size_t elem_size);
static void *vector_untyped_insert(vector(void) * vec, const void *elem,
                                   size_t index, size_t elem_size);
static void *vector_untyped_new(size_t elem_size, size_t capacity);
static void *vector_untyped_pop(vector(void) vec, size_t elem_size);
static void *vector_untyped_push(vector(void) * vec, const void *elem,
                                 size_t elem_size);
static void vector_untyped_remove(vector(void) vec, size_t index,
                                  size_t elem_size);
static void *vector_untyped_resize(vector(void) * vec, size_t new_length,
                                   size_t elem_size);
static void *vector_untyped_set(vector(void) * vec, const void *elem,
                                size_t index, size_t elem_size);
static void *vector_untyped_shrink(vector(void) * vec, size_t elem_size);

/* - FUNCTION DEFINITIONS - */

static inline void vector_untyped_clear(void *const vec,
                                        const size_t elem_size) {
  memset(vec, 0, elem_size * vector_length(vec));
}

static inline void *vector_untyped_copy(const void *const vec,
                                        const size_t elem_size) {
  const size_t ALLOCATION =
      (elem_size * vector_capacity(vec)) + sizeof(vector_header);
  void *vec_copy = malloc(ALLOCATION);
  if (vec_copy != NULL) {
    memcpy(vec_copy, vector_header_const(vec), ALLOCATION);
    vec_copy = (vector_header *)vec_copy + 1;
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
  void *attempt =
      vector_untyped_resize(vec, VEC_EXPANSION_FACTOR * LENGTH, elem_size);
  if (attempt == NULL) {
    attempt = vector_untyped_resize(vec, LENGTH + 1, elem_size);
    if (attempt == NULL) return NULL;
  }
  *vec = attempt;
  return attempt;
}

static inline void vector_untyped_for_each(void *const vec, vec_for_each_op op,
                                           void *const args,
                                           const size_t elem_size) {
  const size_t LENGTH = vector_length(vec);
  size_t i;
  if (args == NULL) {
    void *arg_list[2] = {vec, NULL};
    for (i = 0; i < LENGTH; i++) {
      arg_list[1] = (byte *)vec + (i * elem_size);
      op(arg_list);
    }
  } else {
    void *arg_list[3] = {vec, NULL, args};
    for (i = 0; i < LENGTH; i++) {
      arg_list[1] = (byte *)vec + (i * elem_size);
      op(arg_list);
    }
  }
}

static inline void vector_untyped_for_each_c(const void *const vec,
                                             vec_for_each_op_const op,
                                             const void *const args,
                                             const size_t elem_size) {
  const size_t LENGTH = vector_length(vec);
  size_t i;
  if (args == NULL) {
    const void *arg_list[2] = {vec, NULL};
    for (i = 0; i < LENGTH; i++) {
      arg_list[1] = (const byte *)vec + (i * elem_size);
      op(arg_list);
    }
  } else {
    const void *arg_list[3] = {vec, NULL, args};
    for (i = 0; i < LENGTH; i++) {
      arg_list[1] = (const byte *)vec + (i * elem_size);
      op(arg_list);
    }
  }
}

static inline void *vector_untyped_get(void *const vec, const size_t index,
                                       const size_t elem_size) {
  util_assert(index < vector_length(vec));
  return (byte *)vec + (elem_size * index);
}

static inline size_t vector_untyped_index_of(const void *vec,
                                             const void *const elem,
                                             const size_t elem_size) {
  const size_t LEN = vector_length(vec);
  size_t i;
  for (i = 0; i < LEN; i++) {
    if (memcmp((const byte *)vec + (i * elem_size), elem, elem_size) == 0)
      return i;
  }
  return VEC_BAD_INDEX;
}

static void *vector_untyped_insert(void **const vec, const void *elem,
                                   size_t index, size_t elem_size) {
  const size_t LENGTH = vector_length(*vec);
  byte *dst;
  if (index < LENGTH) {
    vector_untyped_resize(vec, LENGTH + 1, elem_size);
    dst = (byte *)*vec + (elem_size * index);
    memmove(dst + elem_size, dst, elem_size * (LENGTH - index));
  }
  return vector_untyped_set(vec, elem, index, elem_size);
}

static inline void *vector_untyped_new(const size_t elem_size,
                                       const size_t capacity) {
  vector_header *const vec = malloc((elem_size * capacity) + sizeof(vector_header));
  if (vec == NULL) return NULL;
  vec->capacity = capacity;
  vec->length = 0;
  return vec + 1;
}

static inline void *vector_untyped_pop(void *const vec, size_t elem_size) {
  return (byte *)vec + (--vector_header(vec)->length * elem_size);
}

static inline void *vector_untyped_push(void **const vec,
                                        const void *const elem,
                                        const size_t elem_size) {
  return vector_untyped_set(vec, elem, vector_length(*vec), elem_size);
}

static inline void vector_untyped_remove(void *const vec, const size_t index,
                                         const size_t elem_size) {
  byte *const dst = (byte *)vec + (index * elem_size);
  byte *const src = dst + elem_size;
  const size_t SHIFT_SIZE = elem_size * (vector_length(vec) - index - 1);
  util_assert(index < vector_length(vec));
  memmove(dst, src, SHIFT_SIZE);
  vector_header(vec)->length--;
}

static inline void *vector_untyped_resize(void **const vec,
                                          const size_t new_length,
                                          const size_t elem_size) {
  vector_header *header = vector_header(*vec);
  if (new_length <= header->capacity) {
    header->length = new_length;
  } else {
    const size_t ALLOCATION = (elem_size * new_length) + sizeof(vector_header);
    vector_header *const new_header = realloc(header, ALLOCATION);
    if (new_header == NULL) return NULL;
    header = new_header;
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
  const size_t ALLOCATION =
      (elem_size * vector_length(*vec)) + sizeof(vector_header);
  vector_header *shrunk_vec = realloc(vector_header(*vec), ALLOCATION);
  if (shrunk_vec != NULL) {
    shrunk_vec->capacity = shrunk_vec->length;
    *vec = shrunk_vec + 1;
  }
  return shrunk_vec;
}
#endif
