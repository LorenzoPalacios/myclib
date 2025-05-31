#include "stacktests.h"

#include <stddef.h>

#include "../../include/myclib.h"
#include "../../stack/stack.h"
#include "../framework.h"

bool test_stack_copy(void) {
  stack(int) stk1;
  stack(int) stk2;
  stack(int) stk3;

  stk1 = stack_new(int, 3);

  stack_push(stk1, 0);
  stack_push(stk1, 1);
  stack_push(stk1, 2);

  stk2 = stack_copy(stk1);
  stk3 = stack_copy_s(stk1);

  TEST_CASE_ASSERT(stack_capacity(stk1) == stack_capacity(stk2));
  TEST_CASE_ASSERT(stack_height(stk1) == stack_height(stk2));

  TEST_CASE_ASSERT(stack_capacity(stk1) == stack_capacity(stk3));
  TEST_CASE_ASSERT(stack_height(stk1) == stack_height(stk3));

  while (!stack_is_empty(stk1))
    TEST_CASE_ASSERT(stack_pop(stk1) == stack_pop(stk2));

  stack_delete(stk1);
  stack_delete(stk2);
  stack_delete(stk3);
  return true;
}

bool test_stack_expand(void) {
  const size_t INITIAL_CAPACITY = 0;
  stack(int) stk1 = stack_new(int, INITIAL_CAPACITY);
  stack(int) stk2 = stack_copy(stk1);

  TEST_CASE_ASSERT(stack_expand(stk1) != NULL);
  TEST_CASE_ASSERT(stack_capacity(stk1) > INITIAL_CAPACITY);

  TEST_CASE_ASSERT(stack_expand_s(stk2) != NULL);
  TEST_CASE_ASSERT(stack_capacity(stk2) > INITIAL_CAPACITY);

  stack_delete(stk1);
  stack_delete(stk2);
  return true;
}

bool test_stack_new(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_new(int, CAPACITY);

  TEST_CASE_ASSERT(stack_is_empty(stk));
  TEST_CASE_ASSERT(stack_capacity(stk) == CAPACITY);

  stack_delete(stk);
  return true;
}

bool test_stack_peek(void) {
  stack(int) stk = stack_new(int, 1);
  const int VALUE = 1;

  stack_push(stk, VALUE);

  TEST_CASE_ASSERT(stack_peek(stk) == VALUE);
  TEST_CASE_ASSERT(stack_peek(stk) == *(int *)stack_peek_s(stk));

  stack_pop(stk);

  TEST_CASE_ASSERT(stack_peek_s(stk) == NULL);

  stack_delete(stk);
  return true;
}

bool test_stack_pop(void) {
  stack(int) stk = stack_new(int, 1);
  const int VALUE_1 = 1;
  const int VALUE_2 = 2;

  stack_push(stk, VALUE_1);
  stack_push(stk, VALUE_2);

  TEST_CASE_ASSERT(stack_pop(stk) == VALUE_2);
  TEST_CASE_ASSERT(*(int *)stack_pop_s(stk) == VALUE_1);

  TEST_CASE_ASSERT(stack_is_empty(stk));

  stack_delete(stk);
  return true;
}

bool test_stack_push(void) {
  stack(int) stk = stack_new(int, 0);
  const int VALUE = 1;

  stack_push(stk, VALUE);
  stack_push_s(stk, VALUE);

  TEST_CASE_ASSERT(stack_height(stk) == 2);

  while (!stack_is_empty(stk)) TEST_CASE_ASSERT(stack_pop(stk) == VALUE);

  stack_delete(stk);
  return true;
}

bool test_stack_resize(void) {
  const size_t INITIAL_CAPACITY = 3;
  const size_t NEW_CAPACITY = 4;
  stack(int) stk = stack_new(int, INITIAL_CAPACITY);

  stack_resize(stk, NEW_CAPACITY);
  TEST_CASE_ASSERT(stack_capacity(stk) == NEW_CAPACITY);
  {
    size_t i;
    for (i = 0; i < NEW_CAPACITY; i++) stack_push(stk, 1);
    TEST_CASE_ASSERT(stack_height(stk) == NEW_CAPACITY);
  }
  stack_resize_s(stk, INITIAL_CAPACITY);
  TEST_CASE_ASSERT(stack_capacity(stk) == INITIAL_CAPACITY);
  TEST_CASE_ASSERT(stack_height(stk) == stack_capacity(stk));

  stack_delete(stk);
  return true;
}

bool test_stack_shrink(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_new(int, CAPACITY);

  stack_shrink(stk);
  TEST_CASE_ASSERT(stack_height(stk) == stack_capacity(stk));
  TEST_CASE_ASSERT(stack_capacity(stk) == 0);

  stack_push(stk, 1);

  stack_shrink_s(stk);
  TEST_CASE_ASSERT(stack_height(stk) == 1);
  TEST_CASE_ASSERT(stack_capacity(stk) > 0);

  stack_delete(stk);
  return true;
}
