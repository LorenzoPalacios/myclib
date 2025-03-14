#include "framework.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/myclib.h"
#include "stacktests/stacktests.h"

/* - DEFINITIONS - */

#define STRINGIFY(x) #x

#define ARR_LEN(test_arr) (sizeof(test_arr) / sizeof *(test_arr))

#define CONSTRUCT_TEST(test_func) \
  {STRINGIFY(test_func), test_func, sizeof(STRINGIFY(test_func)), -1, false}

#define CONSTRUCT_SUITE(tests) \
  {STRINGIFY(tests), tests, sizeof(STRINGIFY(tests)), ARR_LEN(tests), false}

static test STACK_TESTS[] = {
    CONSTRUCT_TEST(test_stack_copy),   CONSTRUCT_TEST(test_stack_delete),
    CONSTRUCT_TEST(test_stack_expand), CONSTRUCT_TEST(test_stack_init),
    CONSTRUCT_TEST(test_stack_new),    CONSTRUCT_TEST(test_stack_peek),
    CONSTRUCT_TEST(test_stack_pop),    CONSTRUCT_TEST(test_stack_push),
    CONSTRUCT_TEST(test_stack_reset),  CONSTRUCT_TEST(test_stack_resize),
    CONSTRUCT_TEST(test_stack_shrink),
};

test_suite TEST_SUITES[] = {
    CONSTRUCT_SUITE(STACK_TESTS),
};

const size_t NUM_TEST_SUITES = ARR_LEN(TEST_SUITES);

#define DISPLAY_BUF_SIZE (1024)
#define DISPLAY_SUITE_FORMAT "(%c) %s\n"
#define DISPLAY_SUITE_TESTS_FORMAT "%zu. (%c) %s\n"
#define DISPLAY_TEST_FORMAT "(%c) %s\n"
#define SKIP_STATUS_CHR(skippable) ((skippable).skip ? 'x' : '*')

/* - FUNCTIONS - */

void display_all_suites(void) {
  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t suite_i = 0;
  for (; suite_i < NUM_TEST_SUITES; suite_i++) {
    const test_suite CUR_SUITE = TEST_SUITES[suite_i];
    const size_t WRITE_SIZE =
        (size_t)sprintf(buf + buf_i, DISPLAY_SUITE_TESTS_FORMAT, suite_i + 1,
                        SKIP_STATUS_CHR(CUR_SUITE), CUR_SUITE.name);
    if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    buf_i += WRITE_SIZE;
  }
  (void)fputs(buf, stdout);
}

/*
 * Although `display_tests_in_suite` could be used to simplify this function,
 * doing so would cause inefficient buffer handling which may cause visible
 * slowness during output.
 */
void display_all_tests(void) {
  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t suite_i = 0;
  for (; suite_i < NUM_TEST_SUITES; suite_i++) {
    const test_suite CUR_SUITE = TEST_SUITES[suite_i];
    size_t test_i = 0;
    for (; test_i < CUR_SUITE.num_tests; test_i++) {
      const test CUR_TEST = CUR_SUITE.tests[test_i];
      const size_t WRITE_SIZE =
          (size_t)sprintf(buf + buf_i, DISPLAY_SUITE_TESTS_FORMAT, test_i + 1,
                          SKIP_STATUS_CHR(CUR_TEST), CUR_TEST.name);
      if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
        (void)fputs(buf, stdout);
        buf_i = 0;
      }
      buf_i += WRITE_SIZE;
    }
  }
  (void)fputs(buf, stdout);
}

inline void display_suite(const test_suite *const suite) {
  printf(DISPLAY_SUITE_FORMAT, SKIP_STATUS_CHR(*suite), suite->name);
}

inline void display_test(const test *const test) {
  printf(DISPLAY_TEST_FORMAT, SKIP_STATUS_CHR(*test), test->name);
}

void display_tests_in_suite(const test_suite *const suite) {
  const size_t TEST_CNT = suite->num_tests;
  const test *const tests = suite->tests;

  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t tests_i = 0;
  for (; tests_i < TEST_CNT; tests_i++) {
    const test CUR_TEST = tests[tests_i];
    const size_t WRITE_SIZE =
        (size_t)sprintf(buf + buf_i, DISPLAY_SUITE_TESTS_FORMAT, tests_i + 1,
                        SKIP_STATUS_CHR(CUR_TEST), CUR_TEST.name);
    if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    buf_i += WRITE_SIZE;
  }
  (void)fputs(buf, stdout);
}

inline void skip_test(test *const test) { test->skip = true; }

inline test_suite *test_suite_new(const size_t test_capacity) {
  test_suite *const suite = malloc(test_capacity + sizeof(test_suite));
  suite->tests = NULL;
  suite->num_tests = 0;
  return suite;
}

inline void toggle_skip(test *const test) { test->skip = !test->skip; }

inline void unskip_test(test *const test) { test->skip = false; }

void run_test(test *const test) {
  const clock_t START_TIME = clock();
  test->test();
  test->elapsed = clock() - START_TIME;
}
