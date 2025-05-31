#include "stack.h"

#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"

inline void *stack_untyped_copy(const void *const stk,
                                const size_t value_size) {
  const size_t ALLOCATION =
      (value_size * stack_capacity(stk)) + sizeof(stack_header);
  stack_header *const new_stk = malloc(ALLOCATION);
  if (new_stk == NULL) return NULL;
  memcpy(new_stk, const_stack_header(stk), ALLOCATION);
  return new_stk + 1;
}

inline void *stack_untyped_expand(void **const stk, const size_t value_size) {
  const size_t CAPACITY = stack_capacity(*stk);
  void *attempt = NULL;
  if (CAPACITY != 0)
    attempt =
        stack_untyped_resize(stk, STK_EXPANSION_FACTOR * CAPACITY, value_size);
  if (attempt == NULL)
    attempt = stack_untyped_resize(stk, CAPACITY + 1, value_size);
  return attempt;
}

inline bool stack_untyped_is_full(const void *const stk) {
  return stack_height(stk) == stack_capacity(stk);
}

inline void *stack_untyped_new(const size_t capacity, const size_t value_size) {
  stack_header *const stk =
      malloc((value_size * capacity) + sizeof(stack_header));
  if (stk == NULL) return NULL;
  stk->height = 0;
  stk->capacity = capacity;
  return stk + 1;
}

inline void *stack_untyped_peek(void *const stk, const size_t value_size) {
  return stack_is_empty(stk)
             ? NULL
             : (byte *)stk + ((stack_height(stk) - 1) * value_size);
}

inline void *stack_untyped_pop(void *const stk, const size_t value_size) {
  return stack_is_empty(stk)
             ? NULL
             : (byte *)stk + (--(stack_header(stk)->height) * value_size);
}

inline void *stack_untyped_push(void **const stk, const void *const value,
                                const size_t value_size) {
  if (stack_is_full(*stk))
    if (stack_untyped_expand(stk, value_size) == NULL) return NULL;
  memcpy((byte *)*stk + (value_size * stack_height(*stk)), value, value_size);
  stack_header(*stk)->height++;
  return stack_untyped_peek(*stk, value_size);
}

inline void *stack_untyped_resize(void **const stk, const size_t new_capacity,
                                  const size_t value_size) {
  const size_t ALLOCATION = (new_capacity * value_size) + sizeof(stack_header);
  stack_header *const header = realloc(stack_header(*stk), ALLOCATION);
  if (header == NULL) return NULL;
  if (new_capacity < header->height) header->height = new_capacity;
  header->capacity = new_capacity;
  *stk = header + 1;
  return *stk;
}

inline stack(void)
    stack_untyped_shrink(void **const stk, const size_t value_size) {
  return stack_untyped_resize(stk, stack_height(*stk), value_size);
}
