#include "strext.h"

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPANSION_FACTOR (2)
#define BASE_STR_CAPACITY (4096 - sizeof(string) - 1)

/* - INTERNAL UTILITY FUNCTIONS - */

/*
 * Used by constructors and reallocators to calculate how much memory should be
 * allocated for a string.
 */
static inline size_t calc_allocation(const size_t requested_cap) {
  return requested_cap + sizeof(string) + 1;
}

static inline size_t get_capacity(const string *const str) {
  /* Subtracting one since a null terminator must be present in the string. */
  return str->allocation - sizeof(string) - 1;
}

static inline char *get_string_contents(string *const str) {
  return (char *)(str + 1);
}

/*
 * Expands the capacity of `str` to at least `new_capacity` as if expanded by
 * `string_expand()`.
 *
 * \note If the initial expansion fails, this function will expand `str` to
 * `new_capacity` directly instead of emulating the behavior of
 * `string_expand()`.
 */
static inline string *expand_str_to_capacity(string *str,
                                             const size_t new_capacity) {
  const size_t DST_CAPACITY = get_capacity(str);
  const size_t EXPANSION_CNT = new_capacity / (DST_CAPACITY + 1);
  if (EXPANSION_CNT != 0) {
    const size_t EXPANDED_CAPACITY =
        EXPANSION_FACTOR * EXPANSION_CNT * DST_CAPACITY;
    string *expanded_str = string_resize(str, EXPANDED_CAPACITY);
    if (expanded_str == NULL) expanded_str = string_resize(str, new_capacity);
    str = expanded_str;
  }
  return str;
}

static inline string *_find_replace(string *src, const char *const needle,
                                    const char *const replacer,
                                    const size_t needle_len,
                                    const size_t replacer_len) {
  if (needle_len == 0) return src;

  char *const needle_pos = strstr(src->data, needle);
  if (needle_pos != NULL) {
    const size_t REQ_CAPACITY = src->length + replacer_len - needle_len;
    if (src->capacity < REQ_CAPACITY) {
      string *const realloc_str = string_expand(src);
      if (realloc_str == NULL) return NULL;
      src = realloc_str;
    }

    const size_t REMAINDER = src->length - (needle_pos - src->data);
    memmove(needle_pos + replacer_len, needle_pos + needle_len, REMAINDER);
    /* Using `memcpy()` since `strcpy()` appends a null terminator. */
    memcpy(needle_pos, replacer, replacer_len);
  }
  if (replacer_len > needle_len)
    src->length += replacer_len - needle_len;
  else
    src->length -= needle_len - replacer_len;
  src->data[src->length] = '\0';
  return src;
}

static string *_string_insert(string *dst, const char *src, size_t src_len,
                              size_t index) {
  if (index > dst->length) return NULL;

  const size_t REQ_CAPACITY = dst->length + src_len;
  dst = expand_str_to_capacity(dst, REQ_CAPACITY);
  if (dst == NULL) return NULL;

  char *const dst_pos = dst->data + index;
  const size_t REMAINDER = dst->length - index;
  memmove(dst_pos + src_len, dst_pos, REMAINDER);
  memcpy(dst_pos, src, src_len);
  dst->length += src_len;
  dst->data[dst->length] = '\0';
  return dst;
}

/* - LIBRARY FUNCTIONS - */

string *string_append_char(string *dst, const char chr) {
  return string_insert_char(dst, chr, dst->length);
}

string *string_append_str(string *dst, string *const src) {
  return string_insert_str(dst, src, dst->length);
}

string *string_append_raw_str(string *dst, const char *src) {
  return string_insert_raw_str(dst, src, dst->length);
}

string *string_append_int(string *str, long long num) {
  if (num < 0) {
    str = string_append_char(str, '-');
    num = -num;
  }
  return string_append_uint(str, (unsigned long long)num);
}

string *string_append_uint(string *str, const unsigned long long num) {
  const size_t DIGIT_CNT = (size_t)log10l(num);
  const size_t REQ_CAPACITY = str->length + DIGIT_CNT;
  if (str->capacity < REQ_CAPACITY) {
    str = expand_str_to_capacity(str, REQ_CAPACITY);
    if (str == NULL) return NULL;
  }
  sprintf(str->data + str->length, "%llu", num);
  str->length += DIGIT_CNT;
  return str;
}

void string_clear(string *const str) {
  str->length = 0;
  str->data[str->length] = '\0';
}

void string_clear_s(string *const str) {
  memset(str->data, '\0', str->capacity);
  str->length = 0;
}

void _string_delete(string **const str) {
  free(*str);
  *str = NULL;
}

void _string_delete_s(string **const str) {
  memset(*str, 0, (*str)->allocation);
  _string_delete(str);
}

string *string_expand(string *const str) {
  const size_t NEW_CAP =
      (str->capacity > 1) ? EXPANSION_FACTOR * str->capacity : 1;
  return string_resize(str, NEW_CAP);
}

string *string_find_replace_str(string *const src, const string *const needle,
                                const string *const replace) {
  return _find_replace(src, needle->data, replace->data, needle->length,
                       replace->length);
}

string *string_find_replace_raw_str(string *const src, const char *const needle,
                                    const string *const replace) {
  const size_t NEEDLE_LEN = strlen(needle);
  return _find_replace(src, needle, replace->data, NEEDLE_LEN, replace->length);
}

string *string_find_replace_char(string *const src, const char needle,
                                 const string *const replace) {
  return _find_replace(src, &needle, replace->data, 1, replace->length);
}

string *string_insert_char(string *dst, const char chr, const size_t index) {
  return _string_insert(dst, &chr, 1, index);
}

string *string_insert_raw_str(string *dst, const char *const src,
                              const size_t index) {
  const size_t SRC_LEN = strlen(src);
  return _string_insert(dst, src, SRC_LEN, index);
}

string *string_insert_str(string *dst, string *const src, const size_t index) {
  return _string_insert(dst, src->data, src->length, index);
}

string *string_of_capacity(const size_t capacity) {
  const size_t ALLOCATION = calc_allocation(capacity);
  string *const str = malloc(ALLOCATION);
  if (str == NULL) return NULL;
  str->data = get_string_contents(str);
  str->length = 0;
  str->capacity = capacity;
  str->allocation = ALLOCATION;
  str->data[str->length] = '\0';
  return str;
}

string *string_of_char(const char chr) {
  string *str = string_of_capacity(1);
  if (str == NULL) return NULL;
  str = string_append_char(str, chr);
  return str;
}

string *string_of_line_stdin(void) {
  return string_of_stream_delim(stdin, '\n');
}

string *string_new_default(void) {
  return string_of_capacity(BASE_STR_CAPACITY);
}

string *string_of_raw_str(const char *raw_str) {
  string *str = string_new_default();
  if (str == NULL) return NULL;

  while (*raw_str != '\0') {
    string *const realloc_str = string_append(str, *raw_str);
    if (realloc_str == NULL) {
      string_delete(str);
      return NULL;
    }
    str = realloc_str;
    raw_str++;
  }
  return str;
}

string *string_of_stream(FILE *const stream) {
  string *str = string_new_default();

  int c = getc(stream);
  while (c != EOF) {
    string *const realloc_str = string_append(str, (char)c);
    if (realloc_str == NULL) {
      string_delete(str);
      return NULL;
    }
    str = realloc_str;
    c = getc(stream);
  }
  return str;
}

string *string_of_stream_delim(FILE *const stream, const char delim) {
  string *str = string_new_default();

  int c = getc(stream);
  while (c != delim && c != EOF) {
    string *const realloc_str = string_append(str, (char)c);
    if (realloc_str == NULL) {
      string_delete(str);
      return NULL;
    }
    str = realloc_str;
    c = getc(stream);
  }
  return str;
}

string *string_resize(string *str, const size_t new_capacity) {
  const size_t ALLOCATION = calc_allocation(new_capacity);
  str = realloc(str, ALLOCATION);
  if (str == NULL) return NULL;
  str->data = get_string_contents(str);
  str->capacity = new_capacity;
  str->allocation = ALLOCATION;

  if (new_capacity < str->length) {
    str->length = str->capacity;
    str->data[str->length] = '\0';
  }
  return str;
}

string *string_shrink_alloc(string *const str) {
  return string_resize(str, str->length);
}
