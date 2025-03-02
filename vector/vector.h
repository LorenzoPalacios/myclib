#ifndef VECTOR_H
#define VECTOR_H

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)

#if (__STDC_VERSION__ < 202311L)
#include <stdbool.h> /* For C99 to C17. */
#endif

#else

#define inline /* `inline` keyword is not standardized prior to C99. */
/* Boolean type for C95 and below. */
typedef unsigned char bool;
#ifndef true
#define true (1)
#endif
#ifndef false
#define false (0)
#endif

#endif

#include <stddef.h>

/* - DEFINITIONS - */

typedef struct vector *vector;

typedef bool (*for_each_op)(void *);

/* - CONVENIENCE MACROS - */

#define vector_init(type, capacity) \
  vector_init_(sizeof((type)((unsigned char)0)), (size_t)(1 * (capacity)))

/* Using `_Generic` to enforce more type safety in macros. */
#if (__STDC_VERSION__ >= 201112L)
#define vector_add(vec, elem) _Generic(vec, vector: vector_add_)(&(vec), elem)

#define vector_clear(vec) _Generic(vec, vector: vector_clear_)(&(vec))

#define vector_copy(vec) _Generic(vec, vector: vector_copy_)(vec)

#define vector_data(vec) _Generic(vec, vector: vector_data_)(vec)

#define vector_delete(vec) _Generic(vec, vector: vector_delete_)(&(vec))

#define vector_expand(vec) _Generic(vec, vector: vector_expand_)(&(vec))

#define vector_for_each(vec, op) _Generic(vec, vector: vector_for_each_)(vec)

#define vector_get(vec, index) \
  _Generic(vec, vector: vector_get_)(vec, (size_t)(1 * (index)))

#define vector_insert(vec, elem, index) \
  _Generic(vec, vector: vector_insert_)(&(vec), elem, (size_t)(1 * (index)))

#define vector_length(vec) _Generic(vec, vector: vector_length_)(vec)

#define vector_new(data) \
  vector_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

#define vector_resize(vec, new_capacity) \
  _Generic(vec, vector: vector_resize_)(vec, (size_t)(1 * (new_capacity)))

#define vector_set(vec, elem, index) \
  _Generic(vec, vector: vector_set_)(vec, elem, (size_t)(1 * (index)))

#define vector_set_range(vec, elem, start, end)                              \
  _Generic(vec, vector: vector_set_range_)(vec, elem, (size_t)(1 * (start)), \
                                           (size_t)(1 * (end)))

#define vector_shrink_to_fit(vec) vector_shrink_to_fit_(&(vec))
#else

#define vector_add(vec, elem) vector_add_(&(vec), elem)

#define vector_clear(vec) vector_clear_(vec)

#define vector_copy(vec) vector_copy(vec)

#define vector_data(vec) vector_data_(vec)

#define vector_delete(vec) vector_delete_(&(vec))

#define vector_expand(vec) vector_expand_(&(vec))

#define vector_get(vec, index) vector_get_(vec, (size_t)(1 * index))

#define vector_insert(vec, elem, index) \
  vector_insert_(&(vec), elem, (size_t)(1 * (index)))

#define vector_length(vec) vector_length_(vec)

#define vector_new(data) \
  vector_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

#define vector_resize(vec, new_capacity) \
  vector_resize_(&(vec), (size_t)(new_capacity))

#define vector_set(vec, elem, index) vector_set_(vec, elem, (size_t)(1 * index))

#define vector_set_range(vec, elem, start, end) \
  vector_set_range_(vec, elem, (size_t)(1 * start), (size_t)(1 * end))

#define vector_shrink_to_fit(vec) vector_shrink_to_fit_(&(vec))
#endif

/* - FUNCTIONS - */

bool vector_add_(vector *vec, const void *elem);

void vector_clear_(vector vec);

vector vector_copy_(vector vec);

/**
 * @brief The contents of a vector.
 *
 * @param vec The vector whose contents are to be retrieved.
 * @note
 * - Writes made to the contents of a vector through this pointer will not be
 * tracked by the implementation.
 * - The contents of the vector are only well-defined within its length.
 */
void *vector_data_(vector vec);

/**
 * @brief Deletes a vector.
 *
 * @param vec The vector to be deleted.
 */
void vector_delete_(vector *vec);

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
void vector_for_each_(vector vec, for_each_op operation);

/**
 * @brief Returns a pointer to the element at the specified index.
 *
 * @param vec The vector to query.
 * @param index The index of the element.
 * @return A pointer to the element at the specified index.
 */
void *vector_get_(vector vec, size_t index);

/**
 * @brief Creates a new vector with the specified element size and length.
 *
 * @param elem_size The size of each element.
 * @param capacity The initial capacity of the vector.
 * @return An empty vector capable of containing `capacity` elements of size
 * `elem_size`.
 */
vector vector_init_(size_t elem_size, size_t capacity);

/**
 * @brief Inserts an element at the specified index in the vector.
 *
 * @param vec The vector to insert into.
 * @param elem The element to insert.
 * @param index The index at which to insert the element.
 */
bool vector_insert_(vector *vec, const void *elem, size_t index);

size_t vector_length_(vector vec);

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

void vector_set_(vector vec, const void *elem, size_t index);

void vector_set_range_(vector vec, const void *elem, size_t start, size_t end);

/**
 * @brief Shrinks the vector's capacity to fit its capacity.
 *
 * @param vec The vector to shrink.
 * @return `true` if the vector was shrunk; `false` otherwise.
 */
bool vector_shrink_to_fit_(vector *vec);
#endif
