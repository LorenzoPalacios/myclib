#include "framework.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/boolmyclib.h"
#include "../include/compat.h"
#include "stacktests/stacktests.h"

/* - FUNCTION MACROS - */

#define ARR_LEN(test_arr) (sizeof(test_arr) / sizeof *(test_arr))

#define CONSTRUCT_SUITE(tests) \
  {STRINGIFY(tests), tests, sizeof(STRINGIFY(tests)) - 1, ARR_LEN(tests), false}

#define CONSTRUCT_TEST(test_func) \
  {STRINGIFY(test_func), test_func, sizeof(STRINGIFY(test_func)) - 1, -1, false}

#define DISPLAY_NUMBERED_WRITE_SIZE(named_obj, index)           \
  count_digits(index) + (sizeof(DISPLAY_NUMBERED_FORMAT) - 7) + \
      (named_obj).name_len

#define INT_TO_CHAR(c) ((char)((unsigned char)(c)))

#define STRINGIFY(x) #x

/* - EXTERNAL DEFINITIONS - */

/*
 * Keywords should be entirely lowercase.
 * Order matters. See `is_keyword()` and the `input_status` constants.
 */
static const char *const KEYWORDS[] = {
    "all",
    "exit",
    "skip",
};

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
const size_t NUM_KEYWORDS = (sizeof(KEYWORDS) / sizeof *(KEYWORDS));

/* - INTERNAL DEFINITIONS - */

/*
 * This should be enough to hold a string representation of `SIZE_MAX`,
 * any defined keywords, and a null terminator.
 */
#define INPUT_BUF_SIZE (64)

#define DISPLAY_BUF_SIZE (1024)
#define DISPLAY_FORMAT "(%c) %s\n"
#define DISPLAY_NUMBERED_FORMAT "%zu. (%c) %s\n"
#define SKIP_STATUS_CHR(skippable) ((skippable).skip ? 'x' : '*')

const char a[] = "%zu. (%c) %s\n";

/* - INTERNAL FUNCTIONS - */

static inline size_t count_digits(size_t num) {
  size_t i = 0;
  while (i++, num /= 10);
  return i;
}

static inline void discard_line(void) { while (getchar() != '\n'); }

static input_status is_keyword(const char *str) {
  input_status i = 0;
  for (; i < NUM_KEYWORDS; i++) {
    const char *keyword = KEYWORDS[i];
    while (*str == *keyword)
      if (*(str++) == '\0' && *(keyword++) == '\0') return i;
  }
  return STATUS_NO_KEYWORD;
}

static inline void str_lower(char *str) {
  for (; *str != '\0'; str++) *str = INT_TO_CHAR(tolower(*str));
}

/* - EXTERNAL FUNCTIONS - */

void display_suites(void) {
  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t suite_i = 0;
  for (; suite_i < NUM_TEST_SUITES; suite_i++) {
    const test_suite CUR_SUITE = TEST_SUITES[suite_i];
    const size_t WRITE_SIZE = DISPLAY_NUMBERED_WRITE_SIZE(CUR_SUITE, suite_i);
    if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    (void)sprintf(buf + buf_i, DISPLAY_NUMBERED_FORMAT, suite_i,
                  SKIP_STATUS_CHR(CUR_SUITE), CUR_SUITE.name);
    buf_i += WRITE_SIZE;
  }
  (void)fputs(buf, stdout);
}

/*
 * Although `display_suite_tests()` could be used to simplify this function,
 * doing so would cause inefficient buffer handling which may cause visible
 * slowness during output.
 */
void display_tests(void) {
  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t suite_i = 0;
  for (; suite_i < NUM_TEST_SUITES; suite_i++) {
    const test_suite CUR_SUITE = TEST_SUITES[suite_i];

    size_t test_i = 0;
    for (; test_i < CUR_SUITE.num_tests; test_i++) {
      const test CUR_TEST = CUR_SUITE.tests[test_i];
      const size_t WRITE_SIZE = DISPLAY_NUMBERED_WRITE_SIZE(CUR_TEST, test_i);
      if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
        (void)fputs(buf, stdout);
        buf_i = 0;
      }
      (void)sprintf(buf + buf_i, DISPLAY_NUMBERED_FORMAT, test_i,
                    SKIP_STATUS_CHR(CUR_TEST), CUR_TEST.name);
      buf_i += WRITE_SIZE;
    }
  }
  (void)fputs(buf, stdout);
}

inline void display_suite(const test_suite *const suite) {
  printf(DISPLAY_FORMAT, SKIP_STATUS_CHR(*suite), suite->name);
}

inline void display_test(const test *const test) {
  printf(DISPLAY_FORMAT, SKIP_STATUS_CHR(*test), test->name);
}

void display_suite_tests(const test_suite *const suite) {
  const size_t TEST_CNT = suite->num_tests;
  const test *const tests = suite->tests;

  char buf[DISPLAY_BUF_SIZE];
  size_t buf_i = 0;
  size_t tests_i = 0;
  for (; tests_i < TEST_CNT; tests_i++) {
    const test CUR_TEST = tests[tests_i];
    const size_t WRITE_SIZE = DISPLAY_NUMBERED_WRITE_SIZE(CUR_TEST, tests_i);
    if (buf_i + WRITE_SIZE >= DISPLAY_BUF_SIZE) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    (void)sprintf(buf + buf_i, DISPLAY_NUMBERED_FORMAT, tests_i,
                  SKIP_STATUS_CHR(CUR_TEST), CUR_TEST.name);
    buf_i += WRITE_SIZE;
  }
  (void)fputs(buf, stdout);
}

input_status get_input(size_t *const index_output) {
  char buf[INPUT_BUF_SIZE];
  input_status keyword;
  {
    size_t i = 0;
    int chr = getchar();
    while (i < sizeof(buf) - 1 && chr != '\n' && chr != EOF) {
      buf[i++] = INT_TO_CHAR(chr);
      chr = getchar();
    }
    buf[i] = '\0';
    str_lower(buf);
    if (chr != '\n') discard_line();
  }
  keyword = is_keyword(buf);
  if (keyword == STATUS_NO_KEYWORD) {
    if (sscanf(buf, "%zu", index_output) == 1) return STATUS_INDEX;
    return STATUS_BAD_INPUT;
  }
  return keyword;
}

input_status get_selected_suite(const test_suite **const suite_output) {
  size_t suite_index;
  const input_status status = get_input(&suite_index);
  if (status == STATUS_INDEX)
    if (suite_index < NUM_TEST_SUITES)
      *suite_output = TEST_SUITES + suite_index;
  return status;
}

input_status get_selected_test(const test_suite *const suite,
                               const test **const test_output) {
  size_t suite_index;
  const input_status status = get_input(&suite_index);
  if (status == STATUS_INDEX) *test_output = suite->tests + suite_index;
  return status;
}

void run_all_suites(void) {
  size_t i = 0;
  for (; i < NUM_TEST_SUITES; i++) run_suite(TEST_SUITES + i);
}

void run_suite(test_suite *const suite) {
  if (!suite->skip) {
    size_t i = 0;
    for (; i < suite->num_tests; i++) run_test(suite->tests + i);
  }
}

void run_test(test *const test) {
  if (!test->skip) {
    const clock_t START_TIME = clock();
    test->test();
    test->elapsed = clock() - START_TIME;
  }
}

inline void toggle_skip(test *const test) { test->skip = !test->skip; }
