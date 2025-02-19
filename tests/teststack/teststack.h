#ifndef TEST_STACK_H
#define TEST_STACK_H

/* - AVAILABLE TEST FUNCTIONS - */

void test_stack_auto_init(void);

void test_stack_init(void);

void test_stack_new(void);

void test_stack_resize(void);

void test_stack_shrink(void);

/* - DEFINITIONS - */

typedef void (*test_func)(void);

#define STACK_TESTS                                                     \
  ((test_func[]){test_stack_auto_init, test_stack_init, test_stack_new, \
                 test_stack_resize, test_stack_shrink})

#define NUM_STACK_TESTS \
  (sizeof(STACK_TESTS) / sizeof *(STACK_TESTS))
#endif
