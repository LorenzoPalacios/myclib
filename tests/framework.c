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

#define BOOL_AS_CHAR(b) ((b) ? 1 : 0)

#define BOOL_AS_STRING(b) ((b) ? "true" : "false")

#define CLOCK_TO_SEC(n) ((n) / (double)CLOCKS_PER_SEC)

#define CONSTRUCT_SUITE(tests) {STRINGIFY(tests), tests, ARR_LEN(tests), false}

#define CONSTRUCT_TEST(test_func) \
  {STRINGIFY(test_func), test_func, -1, false, false}

#define DISPLAY_WRITE_SIZE(named_obj, index) \
  count_digits(index) + (sizeof(DISPLAY_FORMAT) - 7) + (named_obj).name_len

#define INT_TO_CHAR(c) ((char)(unsigned char)(c))

#define TOGGLE_SKIP(skippable) (skippable).skip = !(skippable).skip

/* Suite index, skip status. */
#define SUITE_CONFIG_FILE_FORMAT "%zu %d\n"

/* Suite index, test index, skip status. */
#define TEST_CONFIG_FILE_FORMAT "%zu %zu %d\n"

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

#define DISPLAY_FORMAT "%zu. (%c) %s\n"

/*
 * This should be enough to hold a string representation of `SIZE_MAX`,
 * any defined keywords, and a null terminator.
 */
#define INPUT_BUF_SIZE (64)

/*
 * Keywords should be entirely lowercase.
 * Order matters. See `is_keyword()` and the `input_status` constants.
 */
static const char *const INPUT_KEYWORDS[] = {
    "all",
    "exit",
    "skip",
};

#define INPUT_SEPARATOR (' ')

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

/* Helper function used by `save_test_config()`. */
static void write_suite_config(test_suite *const suite, void *const output) {
  const size_t SUITE_INDEX = (size_t)(suite - TEST_SUITES);
  size_t i;
  fprintf(output, SUITE_CONFIG_FILE_FORMAT, SUITE_INDEX,
          BOOL_AS_CHAR(suite->skip));
  for (i = 0; i < suite->num_tests; i++) {
    const test *const TEST = suite->tests + i;
    fprintf(output, TEST_CONFIG_FILE_FORMAT, SUITE_INDEX, i,
            BOOL_AS_CHAR(TEST->skip));
  }
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

inline void for_each_suite(const suite_op op, void *args) {
  size_t i;
  for (i = 0; i < NUM_TEST_SUITES; i++) op(TEST_SUITES + i, args);
}

inline void for_each_suite_no_arg(const suite_op_no_arg op) {
  size_t i;
  for (i = 0; i < NUM_TEST_SUITES; i++) op(TEST_SUITES + i);
}

inline void for_each_test(test_suite *const suite, const test_op op,
                          void *args) {
  size_t i;
  for (i = 0; i < suite->num_tests; i++) op(suite->tests + i, args);
}

inline void for_each_test_no_arg(test_suite *const suite,
                                 const test_op_no_arg op) {
  size_t i;
  for (i = 0; i < suite->num_tests; i++) op(suite->tests + i);
}

static size_t parse_config_line(size_t *const data, FILE *const input) {
  size_t i;
  for (i = 0; i < 3; i++) {
    fscanf(input, "%zu", data + i);
    if (fgetc(input) == '\n') break;
  }
  return i;
}

bool load_test_config(void) {
  FILE *const input = fopen(CONFIG_FILENAME, "r");
  if (input == NULL) {
    return false;
  }
  while (true) {
    test_suite *SUITE;
    size_t data[3];
    const size_t writes = parse_config_line(data, input);
    SUITE = TEST_SUITES + data[0];
    if (writes == 1)
      SUITE->skip = data[1];
    else if (writes == 2)
      SUITE->tests[data[1]].skip = data[2];
    else
      break;
  }
  fclose(input);
  return true;
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
  if (!suite->skip) for_each_test_no_arg(suite, run_test);
}

void run_test(test *const test) {
  if (!test->skip) {
    const clock_t START_TIME = clock();
    test->passed = test->test();
    test->elapsed = clock() - START_TIME;
  }
}

bool save_test_config(void) {
  FILE *const output = fopen(CONFIG_FILENAME, "w");
  if (output == NULL) return false;
  setvbuf(output, NULL, _IOFBF, BUFSIZ);
  for_each_suite(write_suite_config, output);
  fclose(output);
  return true;
}

static void write_suite_results(test_suite *const suite, void *const output) {
  size_t i;
  fprintf(output, "%s:\n", suite->name);
  for (i = 0; i < suite->num_tests; i++) {
    const test *CUR_TEST = suite->tests + i;
    fprintf(output,
            "\t- %s:\n"
            "\t\tElapsed time (seconds): %lg\n"
            "\t\tPassed: %s\n"
            "\t\tSkipped: %s\n",
            CUR_TEST->name, CLOCK_TO_SEC(CUR_TEST->elapsed),
            BOOL_AS_STRING(CUR_TEST->passed), BOOL_AS_STRING(CUR_TEST->skip));
  }
}

bool save_test_results(void) {
  FILE *const output = fopen(RESULTS_FILENAME, "w");
  if (output==NULL)return false;
  setvbuf(output, NULL, _IOFBF, BUFSIZ);
  for_each_suite(write_suite_results, output);
  fclose(output);
  return true;
}

inline void skip_all_suites(void) { for_each_suite_no_arg(skip_suite); }

inline void skip_all_tests(test_suite *const suite) {
  for_each_test_no_arg(suite, skip_test);
}

inline void skip_suite(test_suite *const suite) { TOGGLE_SKIP(*suite); }

inline void skip_test(test *const test) { TOGGLE_SKIP(*test); }
