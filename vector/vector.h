#ifndef VECTOR_H
#define VECTOR_H

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)
#if (__STDC_VERSION__ < 202311L)
/* For C99 to C17. */
#include <stdbool.h>
#endif
#else
/* For C95 and below. */
#if (!(defined true || defined false))
typedef unsigned char bool;
#define true (1)
#define false (0)
#endif
#define inline
#endif

#include <stddef.h>

/* - CONVENIENCE MACROS - */

#define vector_clear(vec) vector_clear_(&(vec))

#define vector_delete(vec) vector_delete_(&(vec))

#define vector_expand(vec) vector_expand(&(vec))

#define vector_for_each(vec, op) vector_for_each_(&(vec), op)

#define vector_get(vec, index) vector_get_(&(vec), index)

#define vector_insert(vec, elem, index) vector_insert_(&(vec), elem, index)

#define vector_init(type, length) vector_init_(sizeof(type), length)

#define vector_is_full(vec) vector_is_full_(&(vec))

#define vector_length(vec) vector_length_(&(vec))

#define vector_new(data) \
  vector_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

#define vector_resize(vec, new_capacity) vector_resize_(&(vec), new_capacity)

#define vector_set(vec, elem, index) vector_set_(&(vec), elem, index)

#define vector_shrink_to_fit(vec) vector_shrink_to_fit_(&(vec))

/* - DEFINITIONS - */

/*
 *   `data`    - The contents of the vector.
 *  `length`   - The maximum number of elements that can be held by the vector
 * until expansion is necessary.
 * `capacity`  - The number of elements the vector has allocated for.
 * `elem_size` - The size of each element in the vector.
 */
typedef struct vector {
  void *data;
  size_t length;
  size_t capacity;
  size_t elem_size;
} vector;

typedef bool (*for_each_op)(void *);

/* - FUNCTIONS - */

void vector_clear_(const vector *vec);

/**
 * @brief Deletes a vector.
 *
 * @param vec The vector to be deleted.
 */
void vector_delete_(const vector *vec);

/**
 * @brief Expands the capacity of the vector.
 *
 * @param vec The vector to expand.
 * @return A pointer to the expanded vector.
 */
bool vector_expand_(vector *vec);

/**
 * @brief Applies a function to each element of the vector until either all
 * elements of `vec` are traversed or `operation` returns `false`.
 *
 * @param vec The vector to iterate over.
 * @param operation The function to apply to each element.
 */
void vector_for_each_(vector *vec, for_each_op operation);

/**
 * @brief Returns a pointer to the element at the specified index.
 *
 * @param vec The vector to query.
 * @param index The index of the element.
 * @return A pointer to the element at the specified index.
 */
void *vector_get_(const vector *vec, size_t index);

/**
 * @brief Creates a new vector with the specified element size and length.
 *
 * @param elem_size The size of each element.
 * @param length The initial length of the vector.
 * @return An empty vector containing at
 */
vector vector_init_(size_t elem_size, size_t length);

/**
 * @brief Inserts an element at the specified index in the vector.
 *
 * @param vec The vector to insert into.
 * @param elem The element to insert.
 * @param index The index at which to insert the element.
 */
void vector_insert_(vector *vec, const void *elem, size_t index);

/**
 * @brief Creates a new vector from an array.
 *
 * @param data The array to initialize the vector with.
 * @param elem_size The size of each element.
 * @param length The length of the array.
 * @return A vector whose contents are a copy of `data`.
 */
vector vector_new_(const void *data, size_t elem_size, size_t length);

/**
 * @brief Resizes the vector to the specified capacity.
 *
 * @param vec The vector to resize.
 * @param new_length The new capacity of the vector.
 * @return `true` if the vector was resized; `false` otherwise.
 */
bool vector_resize_(vector *vec, size_t new_length);

void vector_set_(vector *vec, const void *elem, size_t index);

/**
 * @brief Shrinks the vector's capacity to fit its capacity.
 *
 * @param vec The vector to shrink.
 * @return `true` if the vector was shrunk; `false` otherwise.
 */
bool vector_shrink_to_fit_(vector *vec);

#endif
