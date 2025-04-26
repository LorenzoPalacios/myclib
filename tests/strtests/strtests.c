#include "strtests.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/myclib.h"
#include "../../str/str.h"
#include "../framework.h"

static const char TEST_CHARS[] = {
    'a', 'z', '\'', '"', '\\', '%',
};

static const char *const TEST_STRINGS[] = {
    "Hello, world!", "1234567890", "abc123", "", "\0", "\n",
};

#if (IS_STDC99)
static const wb_int TEST_NUMBERS[] = {
    123,      0,        -1,        1,         120,
    LONG_MAX, LONG_MIN, LLONG_MAX, LLONG_MIN, LLONG_MAX >> 1,
};
#else
static const wb_int TEST_NUMBERS[] = {
    123, 0, -1, 1, 120, LONG_MAX, LONG_MIN,
};
#endif

/* - INTERNAL - */

static bool test_string_append_char(void) {
  string str = string_init(0);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_CHARS); i++) {
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_append(str, TEST_CHARS[i]));
#else
    TEST_CASE_ASSERT(string_append_char(str, TEST_CHARS[i]));
#endif
    TEST_CASE_ASSERT(str[i] == TEST_CHARS[i]);
  }
  string_delete(str);
  return true;
}

static bool test_string_append_raw_str(void) {
  string str = string_init(0);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_STRINGS); i++) {
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_append(str, TEST_STRINGS[i]));
#else
    TEST_CASE_ASSERT(string_append_raw_str(str, TEST_STRINGS[i]));
#endif
    TEST_CASE_ASSERT(string_equals(str, TEST_STRINGS[i]));
    string_clear(str);
  }
  string_delete(str);
  return true;
}

static bool test_string_append_int(void) {
  string str = string_init(0);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
    wb_int temp;
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_append(str, TEST_NUMBERS[i]));
#else
    TEST_CASE_ASSERT(string_append_int(str, TEST_NUMBERS[i]));
#endif
    TEST_CASE_ASSERT(sscanf(str, WB_INT_FMT, &temp) != 0);
    TEST_CASE_ASSERT(temp == (wb_int)TEST_NUMBERS[i]);
    string_clear(str);
  }
  string_delete(str);
  return true;
}

static bool test_string_append_uint(void) {
  string str = string_init(0);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
    wb_uint temp;
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_append(str, TEST_NUMBERS[i]));
#else
    TEST_CASE_ASSERT(string_append_uint(str, TEST_NUMBERS[i]));
#endif
    TEST_CASE_ASSERT(sscanf(str, WB_UINT_FMT, &temp) != 0);
    TEST_CASE_ASSERT(temp == (wb_uint)TEST_NUMBERS[i]);
    string_clear(str);
  }
  string_delete(str);
  return true;
}

static bool test_string_ctor_char(void) {
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_CHARS); i++) {
#if (IS_STDC11)
    string str = string(TEST_CHARS[i]);
#else
    string str = string_from_char(TEST_CHARS[i]);
#endif
    TEST_CASE_ASSERT(str != NULL);
    TEST_CASE_ASSERT(string_length(str) == 1);
    TEST_CASE_ASSERT(str[0] == TEST_CHARS[i]);
    string_delete(str);
  }
  return true;
}

static bool test_string_ctor_number(void) {
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
#if (IS_STDC11)
    string str = string(i);
#else
    string str = string_init(i);
#endif
    TEST_CASE_ASSERT(str != NULL);
    TEST_CASE_ASSERT(string_length(str) == 0);
    TEST_CASE_ASSERT(string_capacity(str) == i);
    string_delete(str);
  }
  return true;
}

static bool test_string_ctor_raw_str(void) {
  size_t i;
  for (i = 0; i < ARR_LEN(TEST_STRINGS); i++) {
#if (IS_STDC11)
    string str = string(TEST_STRINGS[i]);
#else
    string str = string_from_raw_str(TEST_STRINGS[i]);
#endif
    TEST_CASE_ASSERT(str != NULL);
    TEST_CASE_ASSERT(string_length(str) == strlen(TEST_STRINGS[i]));
    TEST_CASE_ASSERT(string_equals(str, TEST_STRINGS[i]));
    string_delete(str);
  }
  return true;
}

static bool test_string_insert_char(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_CHARS); i++) {
    const size_t INSERTION_POS = string_length(str) - 1;
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_insert(str, TEST_CHARS[i], INSERTION_POS));
#else
    TEST_CASE_ASSERT(string_insert_char(str, TEST_CHARS[i], INSERTION_POS));
#endif
    TEST_CASE_ASSERT(str[INSERTION_POS] == TEST_CHARS[i]);
  }
  string_delete(str);
  return true;
}

static bool test_string_insert_int(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_NUMBERS); i++) {
    const wb_int NUM = TEST_NUMBERS[i];
    const size_t INSERTION_POS = string_length(str) / 2;
    wb_int temp;
    /*
     * This allows the inserted number to be parsed from the string and back
     * into an integral value to be compared with the original value.
     */
    string_insert_char(str, ' ', INSERTION_POS);
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_insert(str, NUM, INSERTION_POS));
#else
    TEST_CASE_ASSERT(string_insert_int(str, NUM, INSERTION_POS));
#endif
    TEST_CASE_ASSERT(sscanf(str + INSERTION_POS, WB_INT_FMT, &temp) != 0);
    TEST_CASE_ASSERT(temp == NUM);
  }
  string_delete(str);
  return true;
}

static bool test_string_insert_raw_str(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_STRINGS); i++) {
    const size_t INSERTION_POS = string_length(str) - 1;
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_insert(str, TEST_STRINGS[i], INSERTION_POS));
#else
    TEST_CASE_ASSERT(
        string_insert_raw_str(str, TEST_STRINGS[i], INSERTION_POS));
#endif
    TEST_CASE_ASSERT(!strncmp(str + INSERTION_POS, TEST_STRINGS[i],
                              strlen(TEST_STRINGS[i])));
  }
  string_delete(str);
  return true;
}

static bool test_string_insert_uint(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);

  size_t i;
  for (i = 0; i < ARR_LEN(TEST_CHARS); i++) {
    const wb_int NUM = TEST_NUMBERS[i];
    const size_t INSERTION_POS = string_length(str) - 1;
    wb_int temp;
    /*
     * This allow the inserted number to be parsed from the string and back into
     * an integral value to be compared with the original value.
     */
    str[INSERTION_POS] = '\0';
#if (IS_STDC11)
    TEST_CASE_ASSERT(string_insert(str, NUM, INSERTION_POS));
#else
    TEST_CASE_ASSERT(string_insert_int(str, NUM, INSERTION_POS));
#endif
    TEST_CASE_ASSERT(sscanf(str + INSERTION_POS, WB_INT_FMT, &temp) != 0);
    TEST_CASE_ASSERT(temp == NUM);
  }
  string_delete(str);
  return true;
}

/* - EXTERNAL - */

bool test_string_append(void) {
  TEST_CASE_ASSERT(test_string_append_char());
  TEST_CASE_ASSERT(test_string_append_int());
  TEST_CASE_ASSERT(test_string_append_raw_str());
  TEST_CASE_ASSERT(test_string_append_uint());
  return true;
}

bool test_string_clear(void) {
  string str = string_init(0);

  string_append_int(str, 2);
  TEST_CASE_ASSERT(string_length(str) != 0);

  string_clear(str);

  TEST_CASE_ASSERT(string_length(str) == 0);
  TEST_CASE_ASSERT(string_length(str) == strlen(str));
  TEST_CASE_ASSERT(str[0] == '\0');

  string_delete(str);

  return true;
}

bool test_string_copy(void) {
  string str1 = string_from_raw_str(TEST_STRINGS[0]);
  string str2 = string_copy(str1);

  TEST_CASE_ASSERT(string_equals(str1, str2));
  TEST_CASE_ASSERT(string_length(str1) == string_length(str2));
  TEST_CASE_ASSERT(string_capacity(str1) == string_capacity(str2));

  string_delete(str1);
  string_delete(str2);

  return true;
}

bool test_string_ctor(void) {
#if (IS_STDC11)
  string str1;
  string str2;
#if (IS_STDC23)
  str1 = string();
#else
  str1 = string(STR_DEFAULT_CAPACITY);
#endif
  TEST_CASE_ASSERT(str1 != NULL);
  TEST_CASE_ASSERT(string_length(str1) == 0);
  TEST_CASE_ASSERT(string_capacity(str1) == STR_DEFAULT_CAPACITY);
  TEST_CASE_ASSERT(str1[0] == '\0');

  str2 = string(str1);
  TEST_CASE_ASSERT(str2 != NULL);
  TEST_CASE_ASSERT(string_length(str2) == string_length(str1));
  TEST_CASE_ASSERT(string_capacity(str2) == string_capacity(str2));
  TEST_CASE_ASSERT(string_equals(str1, str2));

  string_delete(str1);
  string_delete(str2);
#endif
  TEST_CASE_ASSERT(test_string_ctor_char());
  TEST_CASE_ASSERT(test_string_ctor_number());
  TEST_CASE_ASSERT(test_string_ctor_raw_str());

  return true;
}

bool test_string_delete(void) {
  string str = string_init(2);

  TEST_CASE_ASSERT(str != NULL);
  string_delete(str);
  TEST_CASE_ASSERT(str == NULL);

  return true;
}

bool test_string_equals(void) {
  const char *const raw_str1 = TEST_STRINGS[0];
  const char *const raw_str2 = TEST_STRINGS[1];
  string str1 = string_from_raw_str(raw_str1);
  string str2 = string_from_raw_str(raw_str2);

  TEST_CASE_ASSERT(string_equals(str1, str2) == !strcmp(str1, str2));
  TEST_CASE_ASSERT(string_equals(str1, str1));
  TEST_CASE_ASSERT(string_equals(raw_str1, raw_str1));
  TEST_CASE_ASSERT(string_equals(raw_str1, raw_str2) ==
                   !strcmp(raw_str1, raw_str2));

  string_delete(str1);
  string_delete(str2);

  return true;
}

bool test_string_expand(void) {
  string str1 = string_init(0);
  string str2 = string_from_raw_str(TEST_STRINGS[0]);
  const size_t STR1_INITIAL_CAPACITY = string_capacity(str1);
  const size_t STR2_INITIAL_CAPACITY = string_capacity(str2);
  const size_t STR1_INITIAL_LENGTH = string_length(str1);
  const size_t STR2_INITIAL_LENGTH = string_length(str2);

  TEST_CASE_ASSERT(string_expand(str1));
  TEST_CASE_ASSERT(string_expand(str2));

  TEST_CASE_ASSERT(string_capacity(str1) > STR1_INITIAL_CAPACITY);
  TEST_CASE_ASSERT(string_capacity(str2) > STR2_INITIAL_CAPACITY);

  TEST_CASE_ASSERT(string_length(str1) == STR1_INITIAL_LENGTH);
  TEST_CASE_ASSERT(string_length(str2) == STR2_INITIAL_LENGTH);

  string_delete(str1);
  string_delete(str2);

  return true;
}

bool test_string_find_replace(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);
  string tgt = string_copy(str);
  string repl = string_init(3);

  string_append_int(repl, 123);

  TEST_CASE_ASSERT(string_append_char(str, tgt[0]));
  TEST_CASE_ASSERT(string_find_replace_char(str, tgt[0], repl));
  TEST_CASE_ASSERT(!string_equals(str, raw_str));
  TEST_CASE_ASSERT(strstr(str, repl) != NULL);

  TEST_CASE_ASSERT(!string_find_replace_raw_str(str, tgt, repl));
  TEST_CASE_ASSERT(!string_find_replace_raw_str(str, "0123hi456", repl));
  TEST_CASE_ASSERT(string_find_replace_raw_str(str, repl, str));

  TEST_CASE_ASSERT(!string_find_replace_str(repl, tgt, str));
  TEST_CASE_ASSERT(string_find_replace_str(str, str, repl));
  TEST_CASE_ASSERT(string_equals(str, repl));

  string_delete(str);
  string_delete(tgt);
  string_delete(repl);

  return true;
}

bool test_string_insert(void) {
  TEST_CASE_ASSERT(test_string_insert_char());
  TEST_CASE_ASSERT(test_string_insert_raw_str());
  TEST_CASE_ASSERT(test_string_insert_int());
  TEST_CASE_ASSERT(test_string_insert_uint());
  return true;
}

bool test_string_resize(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);
  const size_t STR_INITIAL_LENGTH = string_length(str);
  const size_t CAPACITY1 = string_capacity(str);
  const size_t CAPACITY2 = STR_INITIAL_LENGTH >> 2;
  const size_t CAPACITY3 = string_capacity(str) << 2;
  const size_t CAPACITY4 = 3000;

  TEST_CASE_ASSERT(string_resize(str, CAPACITY1));
  TEST_CASE_ASSERT(string_capacity(str) == CAPACITY1);
  TEST_CASE_ASSERT(string_length(str) == STR_INITIAL_LENGTH);

  TEST_CASE_ASSERT(string_resize(str, CAPACITY2));
  TEST_CASE_ASSERT(string_capacity(str) == CAPACITY2);
  TEST_CASE_ASSERT(string_capacity(str) == string_length(str));
  TEST_CASE_ASSERT(!string_equals(str, raw_str));

  TEST_CASE_ASSERT(string_resize(str, CAPACITY3));
  TEST_CASE_ASSERT(string_capacity(str) == CAPACITY3);
  TEST_CASE_ASSERT(string_capacity(str) > string_length(str));

  TEST_CASE_ASSERT(string_resize(str, CAPACITY4));
  TEST_CASE_ASSERT(string_capacity(str) == CAPACITY4);

  TEST_CASE_ASSERT(string_resize(str, 400));

  return true;
}

bool test_string_shrink(void) {
  const char *const raw_str = TEST_STRINGS[0];
  string str = string_from_raw_str(raw_str);

  TEST_CASE_ASSERT(string_expand(str));

  TEST_CASE_ASSERT(string_shrink(str));

  TEST_CASE_ASSERT(string_capacity(str) == string_length(str));

  TEST_CASE_ASSERT(string_equals(str, raw_str));

  string_delete(str);

  return true;
}
