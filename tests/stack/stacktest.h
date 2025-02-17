#ifndef TEST_STACK_H
#define TEST_STACK_H

// - AVAILABLE TEST FUNCTIONS -

void test_stack_new_(void);

void test_stack_clear(void);

void test_stack_interface_push(void);

void test_stack_resize(void);

void test_stack_shrink_to_fit(void);

void test_stack_empty_new(void);

void test_stack_new(void);

void test_stack_heapless_empty_new(void);

// - DEFINITIONS -

typedef void (*test_func)(void);

#define STACK_TEST_FUNCTIONS         \
  ((test_func[]){                    \
      test_stack_clear,              \
      test_stack_empty_new,          \
      test_stack_heapless_empty_new, \
      test_stack_new,                \
      test_stack_new_,               \
      test_stack_resize,             \
      test_stack_shrink_to_fit,      \
  })

#define NUM_STACK_TEST_FUNCTIONS \
  (sizeof(EXAMPLE_FUNCTIONS) / sizeof *(EXAMPLE_FUNCTIONS))

#endif
