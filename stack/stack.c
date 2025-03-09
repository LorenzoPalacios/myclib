#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* - DEFINITIONS -*/

/*
 *   `height`   - The current height of a stack.
 *  `capacity`  - The maximum number of values a stack can store before
 *                expansion is necessary.
 */
typedef struct stack {
  size_t height;
  size_t capacity;
} stack;

typedef unsigned char byte;

/* The factor by which to expand a stack's capacity. */
#define EXPANSION_FACTOR (2)

inline void stack_delete_(void **const stk) {
  free(stack_header(*stk));
  *stk = NULL;
}
/*
bool stack_expand_(void **const stk) {*/
  /*
   * Reallocation may fail because the requested memory is too large. In this
   * case, we fall back to the safer, yet generally less efficient option of
   * only allocating enough memory for one new element in the stack.
   *
   * This has the side effect of potentially requiring more later reallocations,
   * but is more likely to ensure stability. Of course, if this also fails, then
   * chances are the system is out of memory, so it's fine to return `NULL`.
   */ /*
  const bool EXPANSION_SUCCESS =
      stack_resize_(stk, EXPANSION_FACTOR * stack_capacity(*stk));
  if (!EXPANSION_SUCCESS) return stack_resize_(stk, stack_capacity(*stk) + 1);
  return true;
}
*/
inline void *stack_header(void *const stk) { return (stack *)stk - 1; }

void *stack_new_(const void *const data, const size_t length,
                 const size_t value_size) {
  stack *const stk = malloc((value_size * length) + sizeof(stack));
  stk->height = stk->capacity = length;
  memcpy(stk + 1, data, value_size * length);
  return stk + 1;
}

void *stack_init_(const size_t value_size, const size_t capacity) {
  stack *const stk = malloc((value_size * capacity) + sizeof(stack));
  stk->height = 0;
  stk->capacity = capacity;
  return stk + 1;
}

/*
bool stack_resize_(void **const stk, const size_t new_capacity) {
  stack *stk_actual = stack_header(*stk);
  if (new_capacity == stk_actual->capacity) return false;
  {
    const size_t ALLOCATION = new_capacity * stk_actual->value_size;
    stack new_stk = realloc(stk_actual, ALLOCATION);
    if (new_stk == NULL) return false;
    stk_actual = new_stk;
    stk_actual->data = stk_actual + 1;
  }
  stk_actual->capacity = new_capacity;
  if (new_capacity < stk_actual->height) stk_actual->height = new_capacity;
  return true;
}

bool stack_shrink_(void **const stk) {
  return stack_resize_(stk, stack_height(*stk));
}
*/
inline void *stack_peek_untyped(void *const stk, const size_t value_size) {
  return stack_is_empty(stk)
             ? NULL
             : (byte *)stk + ((stack_height(stk) - 1) * value_size);
}

inline void *stack_pop_untyped(void *const stk, const size_t value_size) {
  return stack_is_empty(stk) ? NULL
                             : (byte *)stk + (--stack_height(stk) * value_size);
}
/*

bool stack_push_(void **const stk, const void *const value) {
  const size_t HEIGHT = (*stk)->height;
  if (HEIGHT == (*stk)->capacity)
    if (!stack_expand_(stk)) return false;
  memcpy(get_value(*stk, HEIGHT), value, (*stk)->value_size);
  (*stk)->height++;
  return true;
}

bool stack_push_no_resize_(void *const stk, const void *const value) {
  if (stk->height != stk->capacity) {
    memcpy(get_value(stk, stk->height), value, stk->value_size);
    stk->height++;
    return true;
  }
  return false;
}
*/

#include <stdio.h>

int main(void) {
  const int data[] = {1, 2, 3};
  stack(int) stk = stack_new(data);
  while (!stack_is_empty(stk))
    printf("%d ", *stack_pop(stk));
  return 0;
}
