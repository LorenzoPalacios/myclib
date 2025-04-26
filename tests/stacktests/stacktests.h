#ifndef TEST_STACK_H
#define TEST_STACK_H

#include "../../include/myclib.h"

/* - AVAILABLE TEST FUNCTIONS - */

/*
 * Note: If any tested function has an `_s` variant, that variant will also be
 * included in the test.
 */

bool test_stack_copy(void);

bool test_stack_delete(void);

bool test_stack_expand(void);

bool test_stack_init(void);

bool test_stack_new(void);

bool test_stack_peek(void);

bool test_stack_pop(void);

bool test_stack_push(void);

bool test_stack_reset(void);

bool test_stack_resize(void);

bool test_stack_shrink(void);

#endif
