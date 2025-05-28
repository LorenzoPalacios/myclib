#include "stack.h"

#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"

/* - DEFINITIONS -*/

/*
 *   `height`   - The current height of a stack.
 *  `capacity`  - The maximum number of values a stack can store before
 *                expansion is necessary.
 */
typedef struct {
  size_t height;
  size_t capacity;
} stk_header;

inline size_t *stack_capacity_(void *const stk) {
  return &((stk_header *)stack_header(stk))->capacity;
}

inline void stack_delete_(void **const stk) {
  free(stack_header(*stk));
  *stk = NULL;
}

bool stack_expand_(void **const stk, const size_t value_size) {
  /*
   * Reallocation may fail because the requested memory is too large. In this
   * case, we fall back to the safer option of only allocating enough memory for
   * one new value in the stack.
   *
   * This has the side effect of potentially requiring later reallocations, but
   * is more likely to ensure stability. Of course, if this also fails, then
   * chances are the system is out of memory, so it's fine to return `false`.
   */
  const bool EXPANSION_SUCCESS = stack_resize_(
      stk, stack_capacity(*stk) * STK_EXPANSION_FACTOR, value_size);
  if (!EXPANSION_SUCCESS)
    return stack_resize_(stk, stack_capacity(*stk) + 1, value_size);
  return true;
}

inline void *stack_header_(void *const stk) { return (byte *)stk - sizeof(stk_header); }

inline size_t *stack_height_(void *const stk) {
  return &((stk_header *)stack_header(stk))->height;
}

inline bool stack_is_full_(void *const stk) {
  return stack_height(stk) == stack_capacity(stk);
}

void *stack_new_(const void *const data, const size_t length,
                 const size_t value_size) {
  stk_header *const stk = malloc((value_size * length) + sizeof(stk_header));
  if (stk == NULL) return NULL;
  stk->height = stk->capacity = length;
  memcpy(stk + 1, data, value_size * length);
  return stk + 1;
}

void *stack_init_(const size_t value_size, const size_t capacity) {
  stk_header *const stk = malloc((value_size * capacity) + sizeof(stk_header));
  if (stk == NULL) return NULL;
  (void)(stk->height = 0), stk->capacity = capacity;
  return stk + 1;
}

inline void stack_reset_(void *const stk) { stack_height(stk) = 0; }

bool stack_resize_(void **const stk, const size_t new_capacity,
                   const size_t value_size) {
  stk_header *header = stack_header(*stk);
  if (new_capacity == stack_capacity(*stk)) return false;
  {
    const size_t ALLOCATION = (new_capacity * value_size) + sizeof(stk_header);
    stk_header *const new_stk = realloc(header, ALLOCATION);
    if (new_stk == NULL) return false;
    header = new_stk;
    *stk = header + 1;
  }
  header->capacity = new_capacity;
  if (new_capacity < header->height) header->height = new_capacity;
  return true;
}

bool stack_shrink_(void **const stk, const size_t value_size) {
  return stack_resize_(stk, stack_height(*stk), value_size);
}

inline void *stack_peek_untyped(void *const stk, const size_t value_size) {
  return stack_is_empty(stk)
             ? NULL
             : (byte *)stk + ((stack_height(stk) - 1) * value_size);
}

inline void *stack_pop_untyped(void *const stk, const size_t value_size) {
  return stack_is_empty(stk)
             ? NULL
             : (byte *)stk + (--(stack_height(stk)) * value_size);
}

static inline void *get_value(void *const stk, const size_t index,
                              const size_t value_size) {
  return (byte *)stk + (value_size * index);
}

bool stack_push_(void **const stk, const void *const value,
                 const size_t value_size) {
  const size_t HEIGHT = stack_height(*stk);
  if (HEIGHT == stack_capacity(*stk))
    if (!stack_expand_(stk, value_size)) return false;
  memcpy(get_value(*stk, HEIGHT, value_size), value, value_size);
  (stack_height(*stk))++;
  return true;
}
