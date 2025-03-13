#ifndef TEST_STACK_H
#define TEST_STACK_H

/* - AVAILABLE TEST FUNCTIONS - */

/*
 * Note: If any tested function has an `_s` variant, that variant will also be
 * included in the test.
 */

void test_stack_copy(void);

void test_stack_delete(void);

void test_stack_expand(void);

void test_stack_init(void);

void test_stack_new(void);

void test_stack_peek(void);

void test_stack_pop(void);

void test_stack_push(void);

void test_stack_reset(void);

void test_stack_resize(void);

void test_stack_shrink(void);

#endif
