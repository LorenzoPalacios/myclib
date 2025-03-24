#include <stddef.h>
#include <stdio.h>

#include "../include/compat.h"
#include "framework.h"

/* - DEFINITIONS - */

typedef enum MAIN_MENU_STATUS {
  RUN_TESTS = 0x0,
  RUN_TESTS_NO_FAIL = 0x1,
  CONFIG_TESTS = 0x2
} MAIN_MENU_STATUS;

#define BUFFER_SIZE (128)

static const char *const MAIN_MENU_OPTIONS[] = {
    "Run Tests",
    "Run Tests (ignore test failure)",
    "Configure Tests",
};

#define NUM_MAIN_MENU_OPTIONS \
  (sizeof(MAIN_MENU_OPTIONS) / sizeof *(MAIN_MENU_OPTIONS))

/* - FUNCTIONS - */

static size_t count_digits(size_t num) {
  size_t i = 0;
  while ((void)i++, num /= 10);
  return i;
}

static inline void display_legend(void) {
  puts(
      "\n- Legend -\n"
      "(*) - An unskipped item.\n"
      "(x) - A skipped item.\n");
}

/* REWRITE */
static void print_buffered(const char *const *strings, const size_t STR_CNT) {
  char buf[BUFFER_SIZE];
  size_t buf_i = 0;
  size_t str_i = 0;
  for (; str_i < STR_CNT; str_i++) {
    size_t chr_i = 0;
    char chr;
    if (buf_i + count_digits(str_i) + 2 >= sizeof(buf) - 1) {
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    buf_i += (size_t)sprintf(buf + buf_i, "%zu. ", str_i);
    do {
      chr = strings[str_i][chr_i];
      if (buf_i == sizeof(buf) - 1) {
        (void)fputs(buf, stdout);
        buf_i = 0;
      }
      buf[buf_i] = chr;
      (void)buf_i++, chr_i++;
    } while (chr != '\0');
    buf[buf_i - 1] = '\n';
    buf[buf_i] = '\0';
  }
  (void)fputs(buf, stdout);
}

static inline void display_menu(void) {
  puts("\n - Testing Menu -");
  print_buffered(MAIN_MENU_OPTIONS, NUM_MAIN_MENU_OPTIONS);
}

static inline void warn_unrecognized_input(void) {
  puts("Unrecognized input.");
}

static inline void warn_index_out_of_bounds(void) { puts("Invalid index."); }

/* - OPTION GETTERS - */

static void get_test_option(test_suite *const suite) {
  const size_t NUM_TESTS = suite->num_tests;
  display_suite_tests(suite);
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
        return;
      case STATUS_SKIP_ALL:
        skip_all_tests(suite);
        display_suite_tests(suite);
        display_legend();
        break;
      case STATUS_SKIP_INDEX:
        if (option >= NUM_TESTS) {
          warn_index_out_of_bounds();
          break;
        }
        skip_test(suite->tests + option);
        display_suite_tests(suite);
        display_legend();
        break;
      default:
        warn_unrecognized_input();
        break;
    }
  }
}

static void get_suite_option(void) {
  display_suites();
  display_legend();
  while (true) {
    size_t option;
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
        return;
      case STATUS_SKIP_ALL:
        skip_all_suites();
        display_suites();
        display_legend();
        break;
      case STATUS_SKIP_INDEX:
        if (option >= NUM_TEST_SUITES) {
          warn_index_out_of_bounds();
          break;
        }
        skip_suite(TEST_SUITES + option);
        display_suites();
        display_legend();
        break;
      case STATUS_INDEX:
        if (option >= NUM_TEST_SUITES) {
          warn_index_out_of_bounds();
          break;
        }
        get_test_option(TEST_SUITES + option);
        display_suites();
        display_legend();
        break;
      default:
        warn_unrecognized_input();
        break;
    }
  }
}

/* - OPTION PARSERS - */

static inline void parse_main_menu_option(const size_t option) {
  switch (option) {
    case RUN_TESTS:
      run_all_suites();
      break;
    case RUN_TESTS_NO_FAIL:
      /* TBD */
      break;
    case CONFIG_TESTS:
      get_suite_option();
      break;
    default:
      warn_unrecognized_input();
      break;
  }
}

static void main_menu_loop(void) {
  size_t option;
  display_menu();
  while (true) {
    const input_status status = parse_input(&option);
    switch (status) {
      case STATUS_KEYWORD_EXIT:
        return;
      case STATUS_INDEX:
        parse_main_menu_option(option);
        display_menu();
        break;
      default:
        warn_unrecognized_input();
    }
  }
}

int main(void) {
  main_menu_loop();
  return 0;
}
