#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

/**
 * @brief Creates a new vector from a raw C-style array.
 *
 * @param data The array to initialize the vector with.
 * @return A pointer to the newly created vector.
 */
#define vector_new(data) \
  vector_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

typedef struct {
  size_t length;      // The number of elements in the vector.
  size_t elem_size;   // The size of each element in the vector.
  size_t allocation;  // The total allocated memory for the vector.
} vector;

/**
 * @brief Appends an element to the end of the vector.
 *
 * @param vec The vector to append to.
 * @param elem The element to append.
 * @return A pointer to the updated vector.
 */
vector *vector_append(vector *vec, const void *elem);

/**
 * @brief Returns the capacity of the vector.
 *
 * @param vec The vector to query.
 * @return The capacity of the vector.
 */
size_t vector_capacity(const vector *vec);

/**
 * @brief Returns a pointer to the data of the vector.
 *
 * @param vec The vector to query.
 * @return A pointer to the data of the vector.
 */
void *vector_data(vector *vec);

/**
 * @brief Deletes a vector.
 *
 * @param vec The vector to be deleted.
 */
void vector_delete(vector *vec);


/**
 * @brief Expands the capacity of the vector.
 *
 * @param vec The vector to expand.
 * @return A pointer to the expanded vector.
 */
vector *vector_expand(vector *vec);

/**
 * @brief Applies a function to each element of the vector.
 *
 * @param vec The vector to iterate over.
 * @param operation The function to apply to each element.
 */
void vector_for_each(vector *vec, void (*operation)(void *elem));

/**
 * @brief Returns a pointer to the element at the specified index.
 *
 * @param vec The vector to query.
 * @param index The index of the element.
 * @return A pointer to the element at the specified index.
 */
void *vector_get(vector *vec, size_t index);

/**
 * @brief Initializes a new vector with the specified element size and length.
 *
 * @param elem_size The size of each element.
 * @param length The initial length of the vector.
 * @return A pointer to the newly created vector.
 */
vector *vector_init(size_t elem_size, size_t length);

/**
 * @brief Inserts an element at the specified index in the vector.
 *
 * @param vec The vector to insert into.
 * @param elem The element to insert.
 * @param index The index at which to insert the element.
 * @return A pointer to the updated vector.
 */
vector *vector_insert(vector *vec, const void *elem, size_t index);

/**
 * @brief Creates a new vector from an array.
 *
 * @param data The array to initialize the vector with.
 * @param elem_size The size of each element.
 * @param length The length of the array.
 * @return A pointer to the newly created vector.
 */
vector *vector_new_(const void *data, size_t elem_size, size_t length);

/**
 * @brief Resizes the vector to the specified capacity.
 *
 * @param vec The vector to resize.
 * @param new_capacity The new capacity of the vector.
 * @return A pointer to the resized vector.
 */
vector *vector_resize(vector *vec, size_t new_capacity);

/**
 * @brief Shrinks the vector's capacity to fit its length.
 *
 * @param vec The vector to shrink.
 * @return A pointer to the resized vector.
 */
vector *vector_shrink_to_fit(vector *vec);

#endif
