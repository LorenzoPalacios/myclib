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

#define LIST_FORMAT "%zu. "

/*
 * @returns The number of bytes that would be used to write a number as per
 * `LIST_FORMAT`.
 */
#define LIST_FORMAT_WRITE_SIZE(n) (count_digits(n) + sizeof(LIST_FORMAT) - 3)

static inline size_t print_list_strcat(char *const buf, size_t buf_i,
                                       const size_t buf_capacity,
                                       const char *src) {
  while (*src != '\0') {
    if (buf_i == buf_capacity - 1) {
      buf[buf_i] = '\0';
      (void)fputs(buf, stdout);
      buf_i = 0;
    }
    buf[buf_i++] = *(src++);
  }
  buf[buf_i] = '\n';
  return buf_i;
}

static inline size_t print_list_number(char *const buf, size_t buf_i,
                                       const size_t buf_capacity,
                                       const size_t num) {
  if (buf_i + LIST_FORMAT_WRITE_SIZE(num) >= buf_capacity) {
    buf[buf_i] = '\0';
    (void)fputs(buf, stdout);
    buf_i = 0;
  }
  return buf_i + (size_t)sprintf(buf + buf_i, LIST_FORMAT, num);
}

static void print_list(const char *const *strings, const size_t STR_CNT) {
  char buf[BUFFER_SIZE];
  size_t buf_i = 0;
  size_t str_i = 0;
  for (; str_i < STR_CNT; str_i++) {
    /*
     * `print_list_number()` calls `sprintf()` which appends a null terminator
     * that is to be overwritten by `print_list_strcat()`, which itself appends
     * a newline that is NOT to be overwritten, hence why `buf_i` is finally
     * incremented by one despite both helper functions returning the index of
     * their last written character.
     */
    buf_i = print_list_number(buf, buf_i, sizeof(buf) - buf_i, str_i);
    buf_i = print_list_strcat(buf, buf_i, sizeof(buf) - buf_i, strings[str_i]);
    buf_i++;
  }
  buf[buf_i] = '\0';
  (void)fputs(buf, stdout);
}

static inline void display_menu(void) {
  puts("\n - Testing Menu -");
  print_list(MAIN_MENU_OPTIONS, NUM_MAIN_MENU_OPTIONS);
}

static inline void warn_unrecognized_input(void) {
  puts("Unrecognized input.");
}

static inline void warn_index(void) { puts("Invalid index."); }

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
          warn_index();
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
          warn_index();
          break;
        }
        skip_suite(TEST_SUITES + option);
        display_suites();
        display_legend();
        break;
      case STATUS_INDEX:
        if (option >= NUM_TEST_SUITES) {
          warn_index();
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
        break;
      default:
        warn_unrecognized_input();
    }
    display_menu();
  }
}

int main(void) {
  main_menu_loop();
  return 0;
}
