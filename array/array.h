#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#define new_array(data) \
  _new_array(data, sizeof *(data), sizeof(data) / sizeof *(data))
  
#define delete_array(arr) _delete_array(&(arr))

typedef unsigned char byte;

typedef struct {
  byte *data;
  size_t capacity;
  size_t length;
  size_t elem_size;
} array_t;

array_t *_new_array(const void *data, size_t elem_size, size_t length);

void *add_elem(array_t *arr, const void *elem);

void for_each(array_t *arr, void (*op)(void *elem));

void *get_elem(const array_t *arr, size_t index);

void _delete_array(array_t **arr);

void delete_array_s(array_t **arr);

array_t *init_array(const size_t elem_size, const size_t length);

void *insert_elem(array_t *const arr, const void *elem, size_t index);

void wipe_array(array_t *arr);

#endif
