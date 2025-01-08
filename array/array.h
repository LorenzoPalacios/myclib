#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#define array_new(data) \
  array_new_(data, sizeof *(data), sizeof(data) / sizeof *(data))

#define array_delete(arr) array_delete_(&(arr))
#define array_delete_s(arr) array_delete_s_(&(arr))

typedef struct {
  void *data;
  size_t capacity;
  size_t length;
  size_t elem_size;
} array;

/**
 * @brief Adds an element to the array.
 *
 * @param arr The array to add the element to.
 * @param elem The element to add.
 * @return A pointer to the added element, or NULL if the array is full.
 */
void *array_add(array *arr, const void *elem);

/**
 * @brief Clears the array, setting all elements to zero.
 *
 * @param arr The array to clear.
 */
void array_clear(array *arr);

/**
 * @brief Deletes the array and frees its memory.
 *
 * @param arr A pointer to the array to delete.
 */
void array_delete_(array **arr);

/**
 * @brief Securely deletes the array by zeroing its memory before freeing it.
 *
 * @param arr A pointer to the array to delete.
 */
void array_delete_s_(array **arr);

/**
 * @brief Applies a function to each element of the array.
 *
 * @param arr The array to iterate over.
 * @param operation The function to apply to each element.
 */
void array_for_each(array *arr, void (*operation)(void *elem));

/**
 * @brief Gets an element from the array by index.
 *
 * @param arr The array to get the element from.
 * @param index The index of the element to get.
 * @return A pointer to the element, or NULL if the index is out of bounds.
 */
void *array_get(const array *arr, size_t index);

/**
 * @brief Initializes a new array with the specified element size and length.
 *
 * @param elem_size The size of each element in the array.
 * @param length The initial length of the array.
 * @return A pointer to the initialized array, or NULL if allocation fails.
 */
array *array_init(size_t elem_size, size_t length);

/**
 * @brief Inserts an element into the array at the specified index.
 *
 * @param arr The array to insert the element into.
 * @param elem The element to insert.
 * @param index The index to insert the element at.
 * @return A pointer to the inserted element, or NULL if the array is full or
 * the index is out of bounds.
 */
void *array_insert(array *arr, const void *elem, size_t index);

/**
 * @brief Creates a new array from the given data.
 *
 * @param data The data to initialize the array with.
 * @param elem_size The size of each element in the data.
 * @param length The length of the data.
 * @return A pointer to the new array, or NULL if allocation fails.
 */
array *array_new_(const void *data, size_t elem_size, size_t length);

#endif
