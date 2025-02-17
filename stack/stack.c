#include "stack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/* The factor by which to expand a stack's capacity. */
#define EXPANSION_FACTOR (2)

void stack_reset_(stack *const stk) { stk->length = 0; }

void stack_delete_(const stack *const stk) { free(stk->data); }

bool stack_expand_(stack *const stk) {
  /*
   * Reallocation may fail because the requested memory is too large. In this
   * case, we fall back to the safer, yet generally less efficient option of
   * only allocating enough memory for one new element in the stack.
   *
   * This has the side effect of potentially requiring more later reallocations,
   * but is more likely to ensure stability. Of course, if this also fails, then
   * chances are the system is out of memory, so it's fine to return `NULL`.
   */
  const bool EXPANSION_SUCCESS =
      stack_resize_(stk, EXPANSION_FACTOR * stk->capacity);
  if (!EXPANSION_SUCCESS) return stack_resize_(stk, stk->capacity + 1);
  return EXPANSION_SUCCESS;
}

static inline byte *get_value(stack *const stk, const size_t index) {
  return (byte *)stk->data + (index * stk->value_size);
}

stack stack_new_(const void *const data, const size_t length,
                 const size_t value_size) {
  stack stk;
  stk.data = malloc(value_size * length);
  stk.length = stk.capacity = length;
  stk.value_size = value_size;
  memcpy(stk.data, data, value_size * length);
  return stk;
}

stack stack_init_(const size_t value_size, const size_t capacity) {
  stack stk;
  stk.data = malloc(value_size * capacity);
  stk.length = 0;
  stk.capacity = capacity;
  stk.value_size = value_size;
  return stk;
}

bool stack_is_empty_(const stack *const stk) { return stk->length == 0; }

bool stack_resize_(stack *const stk, const size_t new_capacity) {
  if (new_capacity == stk->capacity) return false;
  {
    const size_t ALLOCATION = new_capacity * stk->value_size;
    void *const new_data = realloc(stk->data, ALLOCATION);
    if (new_data == NULL) return false;
    stk->data = new_data;
  }
  stk->capacity = new_capacity;
  if (new_capacity < stk->length) stk->length = new_capacity;
  return true;
}

bool stack_shrink_(stack *const stk) { return stack_resize_(stk, stk->length); }

void *stack_peek_(const stack *const stk) {
  if (stk->length == 0) return NULL;
  /*
   * Subtracting by one since `stk->length` is equivalent to the number of
   * elements within `stk`.
   */
  return (byte *)stk->data + ((stk->length - 1) * stk->value_size);
}

void *stack_pop_(stack *const stk) {
  return stk->length == 0
             ? NULL
             : (byte *)stk->data + (--stk->length * stk->value_size);
}

void stack_push_(stack *const stk, const void *const elem) {
  const size_t LENGTH = stk->length;
  if (LENGTH == stk->capacity) stack_expand_(stk);
  memcpy(get_value(stk, LENGTH), elem, stk->value_size);
  stk->length++;
}

/* - AUTOMATIC STORAGE DURATION STACK - */

void stack_auto_push(stack *const stk, const void *const elem) {
  const size_t LENGTH = stk->length;
  if (LENGTH == stk->capacity) return;
  memcpy(get_value(stk, LENGTH), elem, stk->value_size);
  stk->length++;
}

/* - INTERFACE STACKS - */

stack stack_interface_(void *const data, const size_t length,
                       const size_t value_size) {
  stack stk;
  stk.data = data;
  stk.length = stk.capacity = length;
  stk.value_size = value_size;
  return stk;
}
