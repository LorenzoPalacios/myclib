#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

#define REALLOC_FACTOR (2)

typedef unsigned char byte;

#define new_vector(data) \
  _vector_new(data, sizeof *(data), sizeof(data) / sizeof *(data))

typedef struct {
  byte *data;
  size_t length;
  size_t capacity;
  size_t elem_size;
} vector;

vector *_vector_new(const void *data, size_t elem_size, size_t length);

void vector_delete(vector **v);

void vector_delete_s(vector **v);

vector *vector_expand(vector *vec);

vector *vector_init(size_t elem_size, size_t length);

vector *vector_resize(vector *vec, size_t new_size);
#endif
