#include "strext.h"

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

static inline size_t get_unused_capacity(const string *const str) {
  return get_capacity(str) - str->length;
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

/* - LIBRARY FUNCTIONS - */

string *string_append_char(string *dst, const char appended) {
  if (get_unused_capacity(dst) == 0) {
    string *const realloc_str = string_expand(dst);
    if (realloc_str == NULL) return NULL;
    dst = realloc_str;
  }
  char *const str_contents = get_string_contents(dst);
  str_contents[dst->length] = appended;
  dst->length++;
  str_contents[dst->length] = '\0';
  return dst;
}

string *string_append_str(string *dst, string *const src) {
  const size_t SRC_LEN = src->length;
  {
    const size_t REQ_DST_CAPACITY = dst->length + SRC_LEN;
    dst = expand_str_to_capacity(dst, REQ_DST_CAPACITY);
    if (dst == NULL) return NULL;
  }
  strcpy(dst->data + dst->length, src->data);
  dst->length += SRC_LEN;
  return dst;
}

string *string_append_raw_str(string *dst, const char *src) {
  while (*(src++) != '\0') {
    dst = string_append_char(dst, *src);
    if (dst == NULL) return NULL;
  }
  return dst;
}

void string_clear(string *const str) {
  str->length = 0;
  str->data[str->length] = '\0';
}

void string_clear_s(string *const str) {
  memset(str->data, '\0', str->capacity);
  str->length = 0;
}

void _string_delete(string **const str_obj) {
  free(*str_obj);
  *str_obj = NULL;
}

void _string_delete_s(string **const str_obj) {
  memset(*str_obj, 0, (*str_obj)->allocation);
  _string_delete(str_obj);
}

string *string_expand(string *const str_obj) {
  const size_t NEW_CAP =
      str_obj->capacity > 1 ? EXPANSION_FACTOR * str_obj->capacity : 1;
  return string_resize(str_obj, NEW_CAP);
}

string *string_find_replace(string *hay, const string *const needle,
                            const string *const replace) {
  const size_t NEEDLE_LEN = needle->length;
  const size_t REPLACER_LEN = replace->length;

  /*
   * There exists a bug where if `needle` is a zero-length string (that is, a
   * string containing only a null terminator), `replace` will be prepended
   * to `hay`.
   * Beyond that, this statement only serves to terminate the function if any
   * passed strings are of length 0, since no meaningful operation can be had.
   */
  // if (NEEDLE_LEN == 0) return hay;
  char *const needle_pos = strstr(hay->data, needle->data);
  if (needle_pos != NULL) {
    {
      const size_t BYTES_REQUIRED = REPLACER_LEN - NEEDLE_LEN;
      if (hay->capacity < BYTES_REQUIRED) {
        string *const realloc_str = string_expand(hay);
        if (realloc_str == NULL) return NULL;
        hay = realloc_str;
      }
    }
    const size_t SHIFT_SIZE = hay->length - (needle_pos - hay->data);
    memmove(needle_pos + NEEDLE_LEN, needle_pos, SHIFT_SIZE);
    strcpy(needle_pos, replace->data);
  }
  return hay;
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

  while (*(raw_str++) != '\0') {
    string *const realloc_str = string_append(str, *raw_str);
    if (realloc_str == NULL) {
      free(str);
      return NULL;
    }
    str = realloc_str;
  }
  return str;
}

string *string_of_stream(FILE *const stream) {
  string *str = string_new_default();

  int c = getc(stream);
  while (c != EOF) {
    string *const realloc_str = string_append(str, (char)c);
    if (realloc_str == NULL) {
      free(str);
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
      free(str);
      return NULL;
    }
    str = realloc_str;
    c = getc(stream);
  }
  return str;
}

string *string_resize(string *str, const size_t new_capacity) {
  const size_t ALLOCATION = calc_allocation(new_capacity);
  str = realloc(str, new_capacity);
  if (str == NULL) return NULL;
  str->allocation = ALLOCATION;
  str->capacity = new_capacity;

  if (new_capacity < str->length) {
    char *const str_contents = get_string_contents(str);
    str->length = str->capacity;
    str_contents[str->length] = '\0';
  }
  return str;
}

string *string_shrink_alloc(string *const str_obj) {
  return string_resize(str_obj, str_obj->length);
}

int main(void) {
  string *str = string_new("hey");
  puts(str->data);
  return 0;
}
