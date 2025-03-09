#include "teststack.h"

#include <assert.h>
#include <stddef.h>

#include "../../stack/stack.h"

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
void test_stack_auto_init(void) {
  stack stk = stack_auto_init(stk, int, 5);
  size_t idx = 1;
  for (; idx < stk.capacity; idx++) stack_auto_push(stk, &idx);

  {
    const int random_value = 3;
    stack_auto_push(stk, &random_value);
  }
  for (; idx > 0; idx--) assert((int)idx == *(int *)stack_pop(stk));
}
#endif

void test_stack_init(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_init(int, ARR_LEN(data));
  {
    size_t idx = 0;
    for (; idx < ARR_LEN(data); idx++) stack_push(stk, data + idx);
  }
  {
    size_t idx = 0;
    for (; idx < ARR_LEN(data); idx++)
      assert(*(int *)stack_pop(stk) == data[idx]);
  }
  stack_delete(stk);
}

void test_stack_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_new(data);
  size_t idx = 0;
  for (; idx < ARR_LEN(data); idx++)
    assert(*(int *)stack_pop(stk) == data[idx]);
  stack_delete(stk);
}

void test_stack_resize(void) {
  stack stk = stack_init(int, 0);

  stack_resize(stk, 9999);
  assert(stk.capacity == 9999);

  stack_resize(stk, 0);
  assert(stk.capacity == 0);

  stack_resize(stk, 5);
  assert(stk.capacity == 5);

  stack_resize(stk, 700);
  assert(stk.capacity == 700);

  stack_delete(stk);
}

void test_stack_shrink(void) {
  stack stk = stack_init(int, 100);
  int elem = 0;
  stack_push(stk, &elem);
  elem = 1;
  stack_push(stk, &elem);
  elem = 2;
  stack_push(stk, &elem);
  stack_shrink(stk);
  assert(stk.length == stk.capacity);
  stack_delete(stk);
}
