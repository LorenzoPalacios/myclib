#include "framework.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/myclib.h"

/* - TESTING HEADERS - */

#include "stacktests/stacktests.h"
#include "strtests/strtests.h"
#include "vectortests/vectortests.h"

/* - FUNCTION MACROS - */

#define BOOL_AS_CHAR(b) ((b) ? 1 : 0)

#define BOOL_AS_STRING(b) ((b) ? "true" : "false")

#define CLOCK_TO_SEC(n) ((n) / (double)CLOCKS_PER_SEC)

#define CONSTRUCT_SUITE(tests) {STRINGIFY(tests), tests, ARR_LEN(tests), false}

#define CONSTRUCT_TEST(test_func) \
  {STRINGIFY(test_func), test_func, -1, false, false}

#define SKIP_AS_CHAR(skip) (skip ? LEGEND_SKIPPED : LEGEND_UNSKIPPED)

/* Suite index, skip status. */
#define SUITE_CONFIG_FILE_FORMAT "%zu %d\n"

/* Suite index, test index, skip status. */
#define TEST_CONFIG_FILE_FORMAT "%zu %zu %d\n"

#define TOGGLE_SKIP(skippable) (skippable).skip = !(skippable).skip

/* - TESTS - */

static test stack_tests[] = {
    CONSTRUCT_TEST(test_stack_copy),   CONSTRUCT_TEST(test_stack_expand),
    CONSTRUCT_TEST(test_stack_new),    CONSTRUCT_TEST(test_stack_peek),
    CONSTRUCT_TEST(test_stack_pop),    CONSTRUCT_TEST(test_stack_push),
    CONSTRUCT_TEST(test_stack_resize), CONSTRUCT_TEST(test_stack_shrink),
};

static test str_tests[] = {
    CONSTRUCT_TEST(test_string_append),
    CONSTRUCT_TEST(test_string_clear),
    CONSTRUCT_TEST(test_string_copy),
    CONSTRUCT_TEST(test_string_delete),
    CONSTRUCT_TEST(test_string_equals),
    CONSTRUCT_TEST(test_string_expand),
    CONSTRUCT_TEST(test_string_find_replace),
    CONSTRUCT_TEST(test_string_insert),
    CONSTRUCT_TEST(test_string_ctor),
    CONSTRUCT_TEST(test_string_resize),
    CONSTRUCT_TEST(test_string_shrink),
};

static test vector_tests[] = {
    CONSTRUCT_TEST(test_vector_clear),    CONSTRUCT_TEST(test_vector_copy),
    CONSTRUCT_TEST(test_vector_delete),   CONSTRUCT_TEST(test_vector_expand),
    CONSTRUCT_TEST(test_vector_for_each), CONSTRUCT_TEST(test_vector_get),
    CONSTRUCT_TEST(test_vector_new),      CONSTRUCT_TEST(test_vector_index_of),
    CONSTRUCT_TEST(test_vector_insert),   CONSTRUCT_TEST(test_vector_pop),
    CONSTRUCT_TEST(test_vector_push),     CONSTRUCT_TEST(test_vector_remove),
    CONSTRUCT_TEST(test_vector_reset),    CONSTRUCT_TEST(test_vector_resize),
    CONSTRUCT_TEST(test_vector_set),      CONSTRUCT_TEST(test_vector_shrink),
};

/* - EXTERNAL DEFINITIONS - */

test_suite test_suites[] = {
    CONSTRUCT_SUITE(stack_tests),
    CONSTRUCT_SUITE(str_tests),
    CONSTRUCT_SUITE(vector_tests),
};

const size_t NUM_TEST_SUITES = ARR_LEN(test_suites);

/* - INTERNAL DEFINITIONS - */

#define CONFIG_FILENAME "test.config"

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
    "all", "exit", "skip", "run", "help",
};

#define INPUT_SEPARATOR (' ')

static const char *const MAIN_MENU_OPTIONS[] = {
    "Run Tests",          "Configure",    "Load Configuration",
    "Save Configuration", "Save Results",
};

typedef enum MAIN_MENU_STATUS {
  RUN_TESTS = 0,
  CONFIG_TESTS = RUN_TESTS + 1,
  LOAD_CONFIG = CONFIG_TESTS + 1,
  SAVE_CONFIG = LOAD_CONFIG + 1,
  SAVE_RESULTS = SAVE_CONFIG + 1
} MAIN_MENU_STATUS;

#define NUM_KEYWORDS (sizeof(INPUT_KEYWORDS) / sizeof *(INPUT_KEYWORDS))

#define NUM_MAIN_MENU_OPTIONS \
  (sizeof(MAIN_MENU_OPTIONS) / sizeof *(MAIN_MENU_OPTIONS))

#define RESULTS_FILENAME "test.results"

/* - INTERNAL FUNCTIONS - */

/* - MISCELLANEOUS - */

static inline void print_list(const char *const *strings,
                              const size_t STR_CNT) {
  size_t i;
  for (i = 0; i < STR_CNT; i++) printf("%zu. %s\n", i, strings[i]);
  (void)fflush(stdout);
}

static inline void str_lower(char *str) {
  for (; *str != '\0'; str++) *str = INTEGRAL_CAST(tolower(*str), char);
}

/* - INPUT - */

static void get_input(char *const buf, const size_t buf_size) {
  size_t i = 0;
  int chr = getchar();
  while (i < buf_size && chr != '\n' && chr != EOF) {
    if (isalnum(chr) || chr == INPUT_SEPARATOR) {
      buf[i++] = INTEGRAL_CAST(chr, char);
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
  if (chr != '\n') discard_line(stdin);
}

/* - PARSING - */

static inline bool is_special_keyword(const input_status keyword) {
  return keyword == STATUS_SKIP_ALL || keyword == STATUS_EXIT ||
         keyword == STATUS_ALL;
}

static size_t parse_config_line(size_t *const data, const size_t data_len,
                                FILE *const input) {
  size_t i;
  for (i = 0; i < data_len; i++) {
    (void)fscanf(input, "%zu", data + i);
    if (fgetc(input) == '\n') break;
  }
  return i;
}

/*
 * @returns If a keyword is found in `str` possible return values include the
 * keyword statuses defined in `enum input_status`.
 * Otherwise, `STATUS_NO_KEYWORD`.
 */
static input_status parse_keywords(const char *str) {
  input_status status = STATUS_NONE;
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
      status |= (input_status)(STATUS_ALL << i);
      if (*str == '\0') return status;
      str_checkpoint = str++;
      i = 0;
    } else {
      str = str_checkpoint;
      i++;
    }
  }
  return status;
}

/*
 * @returns `STATUS_INDEX` upon a successful write to `index_output`.
 * Otherwise, `STATUS_NONE`.
 */
static input_status parse_index(const char *str, size_t *const index_output) {
  while (!isdigit(*str) && *str != '\0') str++;
  if (*str == '\0') return STATUS_NONE;
  if (sscanf(str, "%zu", index_output) != 1) return STATUS_NONE;
  return STATUS_INDEX;
}

/* - OUTPUT - */

/* Helper function used by `save_test_config()`. */
static void write_suite_config(test_suite *const suite, void *const output) {
  const size_t SUITE_INDEX = (size_t)(suite - test_suites);
  size_t i;
  (void)fprintf(output, SUITE_CONFIG_FILE_FORMAT, SUITE_INDEX,
                BOOL_AS_CHAR(suite->skip));
  for (i = 0; i < suite->num_tests; i++) {
    const test *const test = suite->tests + i;
    (void)fprintf(output, TEST_CONFIG_FILE_FORMAT, SUITE_INDEX, i,
                  BOOL_AS_CHAR(test->skip));
  }
}

static inline void write_test_results(test *const test, void *const output) {
  /*
   * An elapsed time of -1 means the test has not been run, so we omit it from
   * the output since it may contain misleading data.
   */
  if (test->elapsed != -1)
    (void)fprintf(output,
                  "\t- %s:\n"
                  "\t\tElapsed time (seconds): %lg\n"
                  "\t\tPassed: %s\n"
                  "\t\tSkipped: %s\n",
                  test->NAME, CLOCK_TO_SEC(test->elapsed),
                  BOOL_AS_STRING(test->passed), BOOL_AS_STRING(test->skip));
}

static inline void write_suite_results(test_suite *const suite,
                                       void *const output) {
  (void)fprintf(output, "%s:\n", suite->NAME);
  for_each_test(suite, write_test_results, output);
}

/* - EXTERNAL FUNCTIONS - */

/* - DISPLAY - */

inline void display_legend(void) {
  printf(
      "\n- Legend -\n"
      "(%c) - A skipped item.\n"
      "(%c) - An unskipped item.\n\n",
      LEGEND_SKIPPED, LEGEND_UNSKIPPED);
  (void)fflush(stdout);
}

void display_main_menu(void) {
  puts("\n - Testing Menu -");
  print_list(MAIN_MENU_OPTIONS, NUM_MAIN_MENU_OPTIONS);
  /* `print_list()` calls `fflush()`. */
}

void display_suites(void) {
  size_t i;
  puts("\n - Suites -");
  for (i = 0; i < NUM_TEST_SUITES; i++) {
    const test_suite CUR_SUITE = test_suites[i];
    printf(DISPLAY_FORMAT, i, SKIP_AS_CHAR(CUR_SUITE.skip), CUR_SUITE.NAME);
  }
  (void)fflush(stdout);
}

void display_suite_tests(const test_suite *const suite) {
  size_t i;
  printf("\n - %s -\n", suite->NAME);
  for (i = 0; i < suite->num_tests; i++) {
    const test CUR_TEST = suite->tests[i];
    printf(DISPLAY_FORMAT, i, SKIP_AS_CHAR(CUR_TEST.skip), CUR_TEST.NAME);
  }
  (void)fflush(stdout);
}

static inline void display_suite_info(void) {
  display_suites();
  display_legend();
}

static inline void display_suite_test_info(const test_suite *const suite) {
  display_suite_tests(suite);
  display_legend();
}
/* - FEEDBACK - */

static inline void notify_bad_input(void) {
  puts("Unrecognized input.");
  (void)fflush(stdout);
}

static inline void notify_index(void) {
  puts("Invalid index.");
  (void)fflush(stdout);
}

static inline void notify_ran_suite(const test_suite *const suite) {
  printf("Ran %s\n", suite->NAME);
  (void)fflush(stdout);
}

static inline void notify_ran_test(const test *const test) {
  printf("Ran %s - %s\n", test->NAME, test->passed ? "SUCCESS" : "FAILURE");
  (void)fflush(stdout);
}

#define notify_skipped(skippable)                                 \
  printf("%s %s be skipped during testing.\n", (skippable)->NAME, \
         (skippable)->skip ? "will" : "will no longer");          \
  (void)fflush(stdout)

static inline void notify_skipped_all_suites(void) {
  printf("Toggled skip status for all %zu suite(s).\n", NUM_TEST_SUITES);
  (void)fflush(stdout);
}

static inline void notify_skipped_all_suite_tests(
    const test_suite *const suite) {
  printf("Toggled skip status for all %zu test(s) in %s.\n", suite->num_tests,
         suite->NAME);
  (void)fflush(stdout);
}

/* - UTILITY - */

inline void for_each_suite(const suite_op op, void *args) {
  size_t i;
  for (i = 0; i < NUM_TEST_SUITES; i++) op(test_suites + i, args);
}

inline void for_each_suite_no_arg(const suite_op_no_arg op) {
  size_t i;
  for (i = 0; i < NUM_TEST_SUITES; i++) op(test_suites + i);
}

inline void for_each_test(const test_suite *const suite, const test_op op,
                          void *args) {
  size_t i;
  for (i = 0; i < suite->num_tests; i++) op(suite->tests + i, args);
}

inline void for_each_test_no_arg(const test_suite *const suite,
                                 const test_op_no_arg op) {
  size_t i;
  for (i = 0; i < suite->num_tests; i++) op(suite->tests + i);
}

input_status parse_input(size_t *const index_output) {
  char buf[INPUT_BUF_SIZE];
  get_input(buf, sizeof(buf) - 1);
  {
    const input_status KEYWORD = parse_keywords(buf);
    const input_status INDEX = parse_index(buf, index_output);
    return is_special_keyword(KEYWORD) ? KEYWORD : KEYWORD | INDEX;
  }
}

/* - CONFIGURATION - */

bool load_config(void) {
  FILE *const input = fopen(CONFIG_FILENAME, "r");
  if (input == NULL) return false;
  while (true) {
    test_suite *suite;
    size_t data[3];
    const size_t WRITES = parse_config_line(data, ARR_LEN(data), input);
    suite = test_suites + data[0];
    if (WRITES == 1)
      suite->skip = (bool)data[1];
    else if (WRITES == 2)
      suite->tests[data[1]].skip = (bool)data[2];
    else
      break;
  }
  (void)fclose(input);
  return true;
}

inline bool load_config_verbose(void) {
  const bool LOAD_SUCCESS = load_config();
  if (LOAD_SUCCESS)
    puts("Test configuration loaded from " CONFIG_FILENAME);
  else
    perror("Failed to load test configuration");
  (void)fflush(stdout);
  return LOAD_SUCCESS;
}

inline bool save_config_verbose(void) {
  const bool SAVE_SUCCESS = save_config();
  if (SAVE_SUCCESS)
    puts("Test configuration saved to " CONFIG_FILENAME);
  else
    puts(
        "Failed to save test configuration.\n"
        "Ensure the program has write access to the current directory"
        " and is not inhibited by any security policy.");
  (void)fflush(stdout);
  return SAVE_SUCCESS;
}

bool save_config(void) {
  FILE *const output = fopen(CONFIG_FILENAME, "w");
  if (output == NULL) return false;
  (void)setvbuf(output, NULL, _IOFBF, BUFSIZ);
  for_each_suite(write_suite_config, output);
  (void)fclose(output);
  return true;
}

inline void skip_all_suites(void) { for_each_suite_no_arg(skip_suite); }

inline void skip_suite(test_suite *const suite) { TOGGLE_SKIP(*suite); }

inline void skip_test(test *const test) { TOGGLE_SKIP(*test); }

inline void skip_tests_in_suite(test_suite *const suite) {
  for_each_test_no_arg(suite, skip_test);
}

/* - TEST RUNNERS - */

inline void run_all_suites(void) { for_each_suite_no_arg(run_suite); }

inline void run_suite(test_suite *const suite) {
  if (!suite->skip) for_each_test_no_arg(suite, run_test);
}

void run_test(test *const test) {
  if (!test->skip) {
    const clock_t START_TIME = clock();
    test->passed = test->TEST();
    test->elapsed = clock() - START_TIME;
  }
}

/* - RESULTS - */

bool save_results(void) {
  FILE *const output = fopen(RESULTS_FILENAME, "w");
  if (output == NULL) return false;
  (void)setvbuf(output, NULL, _IOFBF, BUFSIZ);
  for_each_suite(write_suite_results, output);
  (void)fclose(output);
  return true;
}

inline bool save_results_verbose(void) {
  const bool SAVE_SUCCESS = save_results();
  if (SAVE_SUCCESS)
    puts("Results of the most recent test session saved to " RESULTS_FILENAME);
  else
    puts(
        "Failed to save test results\n"
        "Ensure the program has write access to the current directory"
        " and is not inhibited by any security policy.");
  (void)fflush(stdout);
  return SAVE_SUCCESS;
}

/* - OPTION PARSERS - */

static inline void parse_main_menu_option(const size_t option) {
  switch (option) {
    case RUN_TESTS:
      run_all_suites();
      break;
    case CONFIG_TESTS:
      suite_prompt();
      break;
    case LOAD_CONFIG:
      load_config_verbose();
      break;
    case SAVE_CONFIG:
      save_config_verbose();
      break;
    case SAVE_RESULTS:
      save_results_verbose();
      break;
    default:
      notify_bad_input();
      break;
  }
}

static inline void parse_suite_prompt_input(const input_status status,
                                            const size_t option) {
  switch (status) {
    case STATUS_SKIP_ALL:
      skip_all_suites();
      notify_skipped_all_suites();
      break;
    case STATUS_SKIP_INDEX:
      if (option >= NUM_TEST_SUITES) {
        notify_index();
        break;
      }
      skip_suite(test_suites + option);
      notify_skipped(test_suites + option);
      break;
    case STATUS_INDEX:
      if (option >= NUM_TEST_SUITES) {
        notify_index();
        break;
      }
      test_prompt(test_suites + option);
      break;
    case STATUS_RUN_INDEX:
      if (option >= NUM_TEST_SUITES) {
        notify_index();
        break;
      }
      run_suite(test_suites + option);
      notify_ran_suite(test_suites + option);
      break;
    default:
      notify_bad_input();
      break;
  }
}

static inline void parse_test_prompt_input(test_suite *const suite,
                                           const input_status status,
                                           const size_t option) {
  switch (status) {
    case STATUS_EXIT:
      return;
    case STATUS_SKIP_ALL:
      skip_tests_in_suite(suite);
      notify_skipped_all_suite_tests(suite);
      break;
    case STATUS_SKIP_INDEX:
      if ((option) >= suite->num_tests) {
        notify_index();
        break;
      }
      skip_test(suite->tests + option);
      notify_skipped(suite->tests + option);
      break;
    case STATUS_RUN_INDEX:
      if ((option) >= suite->num_tests) {
        notify_index();
        break;
      }
      run_test(suite->tests + option);
      notify_ran_test(suite->tests + option);
      break;
    default:
      notify_bad_input();
      break;
  }
}

/* - PROMPTS - */

void main_menu_prompt(void) {
  display_main_menu();
  while (true) {
    size_t option;
    const input_status STATUS = parse_input(&option);
    switch (STATUS) {
      case STATUS_EXIT:
        return;
      case STATUS_INDEX:
        parse_main_menu_option(option);
        break;
      default:
        notify_bad_input();
    }
    display_main_menu();
  }
}

void test_prompt(test_suite *const suite) {
  display_suite_test_info(suite);
  while (true) {
    size_t option;
    const input_status STATUS = parse_input(&option);
    if (STATUS == STATUS_EXIT) break;
    parse_test_prompt_input(suite, STATUS, option);
    display_suite_test_info(suite);
  }
}

void suite_prompt(void) {
  display_suite_info();
  while (true) {
    size_t option;
    const input_status STATUS = parse_input(&option);
    if (STATUS == STATUS_EXIT) break;
    parse_suite_prompt_input(STATUS, option);
    display_suite_info();
  }
}
