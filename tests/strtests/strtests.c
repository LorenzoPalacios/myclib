#include "strtests.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../include/utilmyclib.h"
#include "../../str/str.h"
#include "../framework.h"

static const char TEST_CHARS[] = {
    'a', 'z', '\'', '"', '\\', '%',
};

static const char *const TEST_STRINGS[] = {
    "Hello, world!", "1234567890", "abc123", "", "\0", "\n",
};

static const long TEST_NUMBERS[] = {
    123, 0, -1, 1, 120, LONG_MAX, LONG_MIN,
};

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
static const long long TEST_LONG_NUMBERS[] = {
    LONG_MAX, LONG_MIN, LLONG_MAX, LLONG_MIN, LLONG_MAX >> 1,
};
#endif

static bool test_string_append_char(void) {
  string str = string_init(0);
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_CHARS); i++) {
    TEST_CASE_ASSERT(string_append_char(str, TEST_CHARS[i]));
    TEST_CASE_ASSERT(str[i] == TEST_CHARS[i]);
  }
  return true;
}

static bool test_string_append_raw_str(void) {
  string str = string_init(0);
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_STRINGS); i++) {
    TEST_CASE_ASSERT(string_append_raw_str(str, TEST_STRINGS[i]));
    TEST_CASE_ASSERT(string_equals(str, TEST_STRINGS[i]));
    string_clear(str);
  }
  return true;
}

static bool test_string_append_int(void) {
  string str = string_init(0);
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
    TEST_CASE_ASSERT(string_append_int(str, TEST_NUMBERS[i]));
    TEST_CASE_ASSERT(strtol(str, NULL, 0) == TEST_NUMBERS[i]);
    string_clear(str);
  }
  return true;
}

static bool test_string_append_uint(void) {
  string str = string_init(0);
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
#if (defined __STD_VERSION__ && __STDC_VERSION >= 201112L)
    TEST_CASE_ASSERT(string_append_uint(str, TEST_NUMBERS[i]));
    TEST_CASE_ASSERT(strtol(str, NULL, 0) == TEST_NUMBERS[i]);
    string_clear(str);
#else
    TEST_CASE_ASSERT(string_append_uint(str, TEST_NUMBERS[i]));
    TEST_CASE_ASSERT(strtol(str, NULL, 0) == TEST_NUMBERS[i]);
    string_clear(str);
#endif
  }
  return true;
}

bool test_string_append(void) {
  TEST_CASE_ASSERT(test_string_append_char());
  TEST_CASE_ASSERT(test_string_append_int());
  TEST_CASE_ASSERT(test_string_append_raw_str());
  TEST_CASE_ASSERT(test_string_append_uint());
  return true;
}

bool test_string_clear(void) { return true; }

bool test_string_copy(void) { return true; }

bool test_string_delete(void) { return true; }

bool test_string_equals(void) { return true; }

bool test_string_expand(void) { return true; }

bool test_string_find_replace(void) { return true; }

bool test_string_insert(void) { return true; }

bool test_string_ctor(void) {
  return true;
}

bool test_string_resize(void) { return true; }

bool test_string_shrink(void) { return true; }
