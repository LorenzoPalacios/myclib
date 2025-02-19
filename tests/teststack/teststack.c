#include "teststack.h"

#include <assert.h>
#include <stddef.h>

#include "../../stack/stack.h"

typedef void (*test_func)(void);

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

void test_stack_auto_init(void) {
  stack stk = stack_auto_init(stk, int, 5);
  for (size_t i = 1; i < stk.capacity; i++) stack_auto_push(stk, &i);
  stack_auto_push(stk, (int[]){-1});
  for (size_t i = stk.length; i > 0; i--)
    assert((int)i == *(int *)stack_pop(stk));
}

void test_stack_init(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_init(int, ARR_LEN(data));
  for (size_t i = 0; i < ARR_LEN(data); i++) stack_push(stk, data + i);

  for (size_t i = 0; i < ARR_LEN(data); i++)
    assert(*(int *)stack_pop(stk) == data[i]);

  stack_delete(stk);
}

void test_stack_new(void) {
  const int data[] = {1, 2, 3, 4, 5};
  stack stk = stack_new(data);
  for (size_t i = 0; i < ARR_LEN(data); i++)
    assert(*(int *)stack_pop(stk) == data[i]);
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
  stack_push(stk, (int[]){3});
  stack_push(stk, (int[]){141});
  stack_push(stk, (int[]){0xd00d});
  stack_shrink(stk);
  assert(stk.length == stk.capacity);
  stack_delete(stk);
}
