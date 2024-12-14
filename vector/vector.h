#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef unsigned char byte;

#define vector_new(data) \
  _vector_new(data, sizeof *(data), sizeof(data) / sizeof *(data))

typedef struct {
  size_t length;
  size_t elem_size;
  size_t allocation;
} vector;

vector *_vector_new(const void *data, size_t elem_size, size_t length);

vector *vector_add_elem(vector *vec, const void *elem);

void vector_delete(vector **v);

void vector_delete_s(vector **v);

vector *vector_expand(vector *vec);

void vector_for_each(vector *vec, void (*op)(void *elem));

size_t vector_get_capacity(const vector *vec);

size_t vector_get_data_alloc(const vector *vec);

vector *vector_init(size_t elem_size, size_t length);

vector *vector_insert_elem(vector *vec, const void *elem, size_t index);

vector *vector_resize(vector *vec, size_t new_size);

vector *vector_shrink_to_fit(vector *vec);
#endif
