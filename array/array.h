#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdbool.h>

#define array_new(data) \
  array_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

#define array_set(arr, elem) array_add_(&(arr), elem)

#define array_clear(arr) array_clear_(&(arr))

#define array_delete(arr) array_delete_(&(arr))

#define array_for_each(arr, op) array_for_each_(&(arr), op)

#define array_get(arr, index) array_get_(&(arr), index)

#define array_init(type, length) array_init_(sizeof(type), length)

#define array_insert(arr, elem) array_insert_(&(arr), elem)

typedef struct {
  void *const data;
  const size_t length;
  const size_t elem_size;
} array;

/**
 * @brief Clears an array, setting all elements to zero.
 *
 * @param arr The array to clear.
 */
void array_clear_(const array *arr);

/**
 * @brief Deletes an array and frees its memory.
 *
 * @param arr A pointer to the array to delete.
 */
void array_delete_(const array *arr);

/**
 * @brief Applies a function to each element of an array.
 *
 * @param arr The array to iterate over.
 * @param operation The function to apply to each element.
 */
void array_for_each_(array *arr, bool (*operation)(void *elem));

/**
 * @brief Gets an element from an array by index.
 *
 * @param arr The array to get the element from.
 * @param index The index of the element to get.
 * @return A pointer to the element, or NULL if the index is out of bounds.
 */
void *array_get_(const array *arr, size_t index);

/**
 * @brief Initializes a new array with the specified element size and length.
 *
 * @param elem_size The size of each element in the array.
 * @param length The initial length of the array.
 * @return A pointer to the initialized array, or NULL if allocation fails.
 */
array array_init_(size_t elem_size, size_t length);

/**
 * @brief Inserts an element into an array at the specified index.
 *
 * @param arr The array to insert the element into.
 * @param elem The element to insert.
 * @param index The index to insert the element at.
 * @note Every element after the insertion index will be shifted over by one
 * index except for the last element, which will be overwritten by the
 * second-to-last element.
 */
void array_insert_(const array *arr, const void *elem, size_t index);

/**
 * @brief Creates a new array from the given data.
 *
 * @param data The data to initialize the array with.
 * @param elem_size The size of each element in the data.
 * @param length The length of the data.
 * @return A new array whose contents are a copy of those at `data`.
 */
array array_new_(const void *data, size_t elem_size, size_t length);

/**
 * @brief Sets an element in an array.
 *
 * @param arr The array to add the element to.
 * @param elem The element to add.
 */
void array_set_(const array *arr, const void *elem, size_t index);

#endif
