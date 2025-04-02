#include "framework.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/compat.h"
#include "stacktests/stacktests.h"

/* - FUNCTION MACROS - */

#define ARR_LEN(test_arr) (sizeof(test_arr) / sizeof *(test_arr))

#define CONSTRUCT_SUITE(tests) \
  {STRINGIFY(tests), tests, sizeof(STRINGIFY(tests)) - 1, ARR_LEN(tests), false}

#define CONSTRUCT_TEST(test_func)    \
  {STRINGIFY(test_func),             \
   test_func,                        \
   sizeof(STRINGIFY(test_func)) - 1, \
   -1,                               \
   false,                            \
   false}

#define DISPLAY_WRITE_SIZE(named_obj, index) \
  count_digits(index) + (sizeof(DISPLAY_FORMAT) - 7) + (named_obj).name_len

#define INT_TO_CHAR(c) ((char)(unsigned char)(c))

#define SKIP_STATUS_CHR(skippable) \
  ((skippable).skip ? LEGEND_SKIPPED : LEGEND_UNSKIPPED)

#define STRINGIFY(x) #x

/* - TESTS - */

static test STACK_TESTS[] = {
    CONSTRUCT_TEST(test_stack_copy),   CONSTRUCT_TEST(test_stack_delete),
    CONSTRUCT_TEST(test_stack_expand), CONSTRUCT_TEST(test_stack_init),
    CONSTRUCT_TEST(test_stack_new),    CONSTRUCT_TEST(test_stack_peek),
    CONSTRUCT_TEST(test_stack_pop),    CONSTRUCT_TEST(test_stack_push),
    CONSTRUCT_TEST(test_stack_reset),  CONSTRUCT_TEST(test_stack_resize),
    CONSTRUCT_TEST(test_stack_shrink),
};

/* - EXTERNAL DEFINITIONS - */

test_suite TEST_SUITES[] = {
    CONSTRUCT_SUITE(STACK_TESTS),
};

const size_t NUM_TEST_SUITES = ARR_LEN(TEST_SUITES);

/* - INTERNAL DEFINITIONS - */

/*
 * This should be enough to hold a string representation of `SIZE_MAX`,
 * any defined keywords, and a null terminator.
 */
#define INPUT_BUF_SIZE (64)

#define INPUT_SEPARATOR (' ')

#define DISPLAY_FORMAT "%zu. (%c) %s\n"

/*
 * Keywords should be entirely lowercase.
 * Order matters. See `is_keyword()` and the `input_status` constants.
 */
static const char *const INPUT_KEYWORDS[] = {
    "all",
    "exit",
    "skip",
};

#define NUM_KEYWORDS (sizeof(INPUT_KEYWORDS) / sizeof *(INPUT_KEYWORDS))

/* - INTERNAL FUNCTIONS - */

/* This declaration exists for `get_input()`. */
static void str_lower(char *str);

static inline void discard_line(void) { while (getchar() != '\n'); }

static void get_input(char *const buf, const size_t buf_size) {
  size_t i = 0;
  int chr = getchar();
  while (i < buf_size && chr != '\n' && chr != EOF) {
    if (isalnum(chr) || chr == INPUT_SEPARATOR) {
      buf[i++] = INT_TO_CHAR(chr);
    } else {
      /* Discard the rest of the input until the next token. */
      while (!isspace(chr = getchar()));
      /*
       * Once the above loop breaks, `chr` will be some character beginning the
       * next token, so we continue instead of allowing normal control flow.
       */
      continue;
    }
    chr = getchar();
  }
  buf[i] = '\0';
  str_lower(buf);
  if (chr != '\n') discard_line();
}

/*
 * @returns If a keyword is found in `str` possible return values include the
 * keyword statuses defined in `enum input_status`.
 * Otherwise, `STATUS_NO_KEYWORD`.
 */
static input_status parse_keywords(const char *str) {
  input_status status = STATUS_NO_KEYWORD;
  const char *str_checkpoint = str;
  size_t i = 0;
  while (i < NUM_KEYWORDS) {
    const char *keyword = INPUT_KEYWORDS[i];
    while (*str == *keyword && *keyword != '\0') (void)(str++), keyword++;
    if ((*str == INPUT_SEPARATOR || *str == '\0') && *keyword == '\0') {
      /*
       * The following line is dependent on the ordering of the `input_status`
       * enum.
       * The cast is present to undo any implicit promotions from the left
       * shift operator.
       */
      status |= (input_status)(STATUS_KEYWORD_ALL << i);
      if (*str == '\0') return status;
      str_checkpoint = ++str;
      i = 0;
    } else {
      str = str_checkpoint;
      i++;
    }
  }
  return status;
}

static inline bool is_special_keyword(const input_status keyword) {
  return keyword == STATUS_SKIP_ALL || keyword == STATUS_KEYWORD_EXIT ||
         keyword == STATUS_KEYWORD_ALL;
}

/*
 * @returns `STATUS_INDEX` upon a successful write to `index_output`.
 * Otherwise, `STATUS_INVALID`.
 */
static bool parse_index(const char *str, size_t *const index_output) {
  while (!isdigit(*str) && *str != '\0') str++;
  if (*str == '\0') return STATUS_INVALID;
  if (sscanf(str, "%zu", index_output) != 1) return STATUS_INVALID;
  return STATUS_INDEX;
}

static inline void str_lower(char *str) {
  for (; *str != '\0'; str++) *str = INT_TO_CHAR(tolower(*str));
}

/* - EXTERNAL FUNCTIONS - */

inline void display_legend(void) {
  printf(
      "\n- Legend -\n"
      "(%c) - A skipped item.\n"
      "(%c) - An unskipped item.\n\n",
      LEGEND_SKIPPED, LEGEND_UNSKIPPED);
  fflush(stdout);
}

void display_suites(void) {
  size_t i;
  puts("\n - Suites -");
  for (i = 0; i < NUM_TEST_SUITES; i++) {
    const test_suite CUR_SUITE = TEST_SUITES[i];
    printf(DISPLAY_FORMAT, i, SKIP_STATUS_CHR(CUR_SUITE), CUR_SUITE.name);
  }
  (void)fflush(stdout);
}

void display_tests(const test_suite *const suite) {
  size_t i;
  printf("\n - %s -\n", suite->name);
  for (i = 0; i < suite->num_tests; i++) {
    const test CUR_TEST = suite->tests[i];
    printf(DISPLAY_FORMAT, i, SKIP_STATUS_CHR(CUR_TEST), CUR_TEST.name);
  }
  (void)fflush(stdout);
}

input_status parse_input(size_t *const index_output) {
  char buf[INPUT_BUF_SIZE];
  get_input(buf, sizeof(buf) - 1);
  {
    const input_status keyword = parse_keywords(buf);
    const input_status index_status = parse_index(buf, index_output);
    return is_special_keyword(keyword) ? keyword : keyword | index_status;
  }
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
    test->passed = test->test();
    test->elapsed = clock() - START_TIME;
  }
}

void skip_all_suites(void) {
  size_t i = 0;
  for (; i < NUM_TEST_SUITES; i++) TEST_SUITES[i].skip = !TEST_SUITES[i].skip;
}

void skip_all_tests(test_suite *const suite) {
  size_t i = 0;
  for (; i < suite->num_tests; i++)
    suite->tests[i].skip = !suite->tests[i].skip;
}

inline void skip_suite(test_suite *const suite) { suite->skip = !suite->skip; }

inline void skip_test(test *const test) { test->skip = !test->skip; }
