#include "stacktests.h"

#include <stddef.h>
#include <stdio.h>

#include "../../stack/stack.h"
#include "../framework.h"

static const int TEST_ARR[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

static const size_t TEST_ARR_LEN = sizeof(TEST_ARR) / sizeof *(TEST_ARR);

bool test_stack_copy(void) {
  stack(int) stk1 = stack_new(TEST_ARR);
  stack(int) stk1_copy = stack_copy(stk1);

  TEST_SUITE_ASSERT(stack_capacity(stk1) == stack_capacity(stk1_copy));
  TEST_SUITE_ASSERT(stack_height(stk1) == stack_height(stk1_copy));
  TEST_SUITE_ASSERT(stack_height(stk1) == TEST_ARR_LEN);

  while (!stack_is_empty(stk1)) {
    const int num = *stack_pop(stk1);
    const int num_copy = *stack_pop(stk1_copy);
    TEST_SUITE_ASSERT(num == num_copy);
  }

  stack_delete(stk1);
  stack_delete(stk1_copy);
  return true;
}

bool test_stack_delete(void) {
  stack(int) stk1 = stack_init(int, 3);
  stack(int) stk2 = stack_copy(stk1);

  TEST_SUITE_ASSERT(stk1 != NULL);
  stack_delete(stk1);
  TEST_SUITE_ASSERT(stk1 == NULL);
  stack_delete_s(stk2);
  TEST_SUITE_ASSERT(stk2 == NULL);

  return true;
}

bool test_stack_expand(void) {
  const size_t INITIAL_CAPACITY = 0;
  stack(int) stk1 = stack_init(int, INITIAL_CAPACITY);
  stack(int) stk2 = stack_copy(stk1);

  stack_expand(stk1);
  TEST_SUITE_ASSERT(stack_capacity(stk1) > INITIAL_CAPACITY);
  stack_expand(stk2);
  TEST_SUITE_ASSERT(stack_capacity(stk2) > INITIAL_CAPACITY);

  stack_delete(stk1);
  stack_delete(stk2);
  return true;
}

bool test_stack_init(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_init(int, CAPACITY);

  TEST_SUITE_ASSERT(stack_capacity(stk) == CAPACITY);
  TEST_SUITE_ASSERT(stack_peek(stk) == NULL);
  TEST_SUITE_ASSERT(stack_is_empty(stk));

  stack_delete(stk);
  return true;
}

bool test_stack_new(void) {
  stack(int) stk = stack_new(TEST_ARR);

  size_t i = TEST_ARR_LEN - 1;
  for (; !stack_is_empty(stk); i--) {
    const int stk_value = *stack_pop(stk);
    TEST_SUITE_ASSERT(stk_value == TEST_ARR[i]);
  }

  stack_delete(stk);
  return true;
}

bool test_stack_peek(void) {
  stack(int) stk1 = stack_new(TEST_ARR);
  stack(int) stk2 = stack_init(int, 1);

  TEST_SUITE_ASSERT(*stack_peek(stk1) == TEST_ARR[TEST_ARR_LEN - 1]);
  TEST_SUITE_ASSERT(stack_peek_s(stk2) == NULL);

  stack_delete(stk1);
  stack_delete(stk2);
  return true;
}

bool test_stack_pop(void) {
  stack(int) stk1 = stack_new(TEST_ARR);
  stack(int) stk2 = stack_init(int, 1);

  const int stk1_value = *stack_pop(stk1);
  const int *const stk2_value = stack_pop_s(stk2);
  TEST_SUITE_ASSERT(stk1_value == TEST_ARR[TEST_ARR_LEN - 1]);
  TEST_SUITE_ASSERT(stk2_value == NULL);

  stack_delete(stk1);
  stack_delete(stk2);
  return true;
}

bool test_stack_push(void) {
  stack(int) stk = stack_init(int, 0);
  const size_t INITIAL_HEIGHT = stack_height(stk);

  {
    size_t i = 0;
    for (; i < TEST_ARR_LEN; i++) stack_push(stk, TEST_ARR + i);
  }

  TEST_SUITE_ASSERT(stack_height(stk) != INITIAL_HEIGHT);

  {
    size_t i = 0;
    for (; i < TEST_ARR_LEN; i++) stack_push_s(stk, TEST_ARR + i);
  }

  while (!stack_is_empty(stk)) {
    size_t i = 0;
    for (; i < TEST_ARR_LEN; i++) {
      const int stk_value = *stack_pop(stk);
      TEST_SUITE_ASSERT(stk_value == TEST_ARR[TEST_ARR_LEN - i - 1]);
    }
  }

  stack_delete(stk);
  return true;
}

bool test_stack_reset(void) {
  stack(int) stk1 = stack_new(TEST_ARR);

  TEST_SUITE_ASSERT(!stack_is_empty(stk1));
  stack_reset(stk1);
  TEST_SUITE_ASSERT(stack_is_empty(stk1));
  stack_delete(stk1);
  return true;
}

bool test_stack_resize(void) {
  const size_t INITIAL_CAPACITY = 3;
  const size_t NEW_CAPACITY = 4;
  stack(int) stk = stack_init(int, INITIAL_CAPACITY);

  stack_resize(stk, NEW_CAPACITY);
  TEST_SUITE_ASSERT(stack_capacity(stk) == NEW_CAPACITY);
  {
    int i = 0;
    for (; (size_t)i < NEW_CAPACITY; i++)
      TEST_SUITE_ASSERT(stack_push_s(stk, &i));
  }

  stack_delete(stk);
  return true;
}

bool test_stack_shrink(void) {
  const size_t CAPACITY = 3;
  stack(int) stk = stack_init(int, CAPACITY);

  stack_shrink(stk);
  TEST_SUITE_ASSERT(stack_capacity(stk) == 0);

  stack_expand(stk);

  stack_shrink_s(stk);
  TEST_SUITE_ASSERT(stack_capacity(stk) == 0);

  stack_delete(stk);
  return true;
}
