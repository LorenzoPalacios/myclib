#include "str.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPANSION_FACTOR (2)
#define DEFAULT_CAPACITY (4096)

/* - INTERNAL - */

/* Helper function used by integer insertion functions. */
static inline size_t count_digits(size_t num) {
  size_t digits = 1;
  for (; num != 0; digits++) num /= 10;
  return digits;
}

/**
 * @returns `true` if `str->capacity` is at least `new_capacity` before or after
 * expansion. `false` if expansion failed.
 */
static inline bool string_expand_to_capacity(string *const str,
                                             const size_t new_capacity) {
  return str->capacity >= new_capacity ? true
                                       : string_resize(str, new_capacity);
}

static bool find_replace_(string *const src, const char *const tgt,
                          const size_t tgt_len, const char *const repl,
                          const size_t repl_len) {
  if (tgt_len != 0 && tgt_len <= src->length) {
    char *const needle_pos = strstr(src->data, tgt);
    if (needle_pos != NULL) {
      const size_t NEW_LEN = src->length + repl_len - tgt_len;
      const size_t REMAINDER = src->length - (size_t)(needle_pos - src->data);
      if (!string_expand_to_capacity(src, NEW_LEN)) return false;
      memmove(needle_pos + repl_len, needle_pos + tgt_len, REMAINDER);
      /* Using `memcpy()` since `strcpy()` appends a null terminator. */
      memcpy(needle_pos, repl, repl_len);
      src->length = NEW_LEN;
    }
    src->data[src->length] = '\0';
    return true;
  }
  return false;
}

static bool string_insert_(string *const dst, const size_t index,
                           const char *const src, const size_t src_len) {
  const size_t NEW_LEN = dst->length + src_len;
  char *const dst_pos = dst->data + index;
  const size_t REMAINDER = dst->length - index;
  if (index >= dst->length) return false;
  if (!string_expand_to_capacity(dst, NEW_LEN)) return false;
  memmove(dst_pos + src_len, dst_pos, REMAINDER);
  memcpy(dst_pos, src, src_len);
  dst->length = NEW_LEN;
  dst->data[dst->length] = '\0';

  return true;
}

/* - LIBRARY FUNCTIONS - */

bool string_append_char(string *const dst, const char chr) {
  return string_insert_char(dst, chr, dst->length);
}

bool string_append_str(string *const dst, const string *const src) {
  return string_insert_str(dst, src, dst->length);
}

bool string_append_raw_str(string *const dst, const char *src) {
  return string_insert_raw_str(dst, src, dst->length);
}

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)

bool string_append_int(string *const str, long long num) {
  if (num < 0) {
    if (!string_append_char(str, '-')) return false;
    num = -num;
  }
  return string_append_uint(str, (unsigned long long)num);
}

bool string_append_uint(string *const str, const unsigned long long num) {
  const size_t DIGIT_CNT = num == 0 ? 1 : count_digits(num) + 1;
  const size_t NEW_LEN = str->length + DIGIT_CNT;
  if (!string_expand_to_capacity(str, NEW_LEN)) return false;
  (void)sprintf(str->data + str->length, "%llu", num);
  str->length = NEW_LEN;
  return true;
}

#else

bool string_append_int(string *const str, long num) {
  if (num < 0) {
    if (!string_append_char(str, '-')) return false;
    num = -num;
  }
  return string_append_uint(str, (unsigned long)num);
}

bool string_append_uint(string *const str, const unsigned long num) {
  const size_t DIGIT_CNT = num == 0 ? 1 : count_digits(num) + 1;
  const size_t NEW_LEN = str->length + DIGIT_CNT;
  if (!string_expand_to_capacity(str, NEW_LEN)) return false;
  (void)sprintf(str->data + str->length, "%lu", num);
  str->length = NEW_LEN;
  return true;
}
#endif

void string_clear(string *const str) {
  str->length = 0;
  str->data[str->length] = '\0';
}

void string_delete(const string *const str) { free(str->data); }

bool string_expand(string *const str) {
  const size_t CUR_CAP = str->capacity;
  const bool EXPANSION_SUCCESS = string_resize(str, EXPANSION_FACTOR * CUR_CAP);
  return EXPANSION_SUCCESS ? true : string_resize(str, CUR_CAP + 1);
}

bool string_find_replace_str(string *const src, const string *const tgt,
                             const string *const repl) {
  return find_replace_(src, tgt->data, tgt->length, repl->data, repl->length);
}

bool string_find_replace_raw_str(string *const src, const char *const tgt,
                                 const string *const repl) {
  const size_t TGT_LEN = strlen(tgt);
  return find_replace_(src, tgt, TGT_LEN, repl->data, repl->length);
}

bool string_find_replace_char(string *const src, const char tgt,
                              const string *const repl) {
  return find_replace_(src, &tgt, 1, repl->data, repl->length);
}

bool string_insert_char(string *const dst, const char chr, const size_t index) {
  return string_insert_(dst, index, &chr, 1);
}

bool string_insert_raw_str(string *dst, const char *const src,
                           const size_t index) {
  const size_t SRC_LEN = strlen(src);
  return string_insert_(dst, index, src, SRC_LEN);
}

bool string_insert_str(string *const dst, const string *const src,
                       const size_t index) {
  return string_insert_(dst, index, src->data, src->length);
}

string string_init(const size_t capacity) {
  string str;
  str.data = malloc(capacity);
  str.data[0] = '\0';
  str.length = 0;
  str.capacity = capacity;
  return str;
}

string string_new_char(const char chr) {
  string str = string_init(DEFAULT_CAPACITY);
  str.data[0] = chr;
  str.data[1] = '\0';
  str.length = 1;
  return str;
}

string string_new_stdin(void) { return string_new_stream_delim(stdin, '\n'); }

string string_new_c_str(const char *c_str) {
  string str = string_init(DEFAULT_CAPACITY);
  string *const str_ptr = &str;
  for (; *c_str != '\0'; c_str++) string_append_char(str_ptr, *c_str);
  return str;
}

string string_new_stream(FILE *const stream) {
  string str = string_init(DEFAULT_CAPACITY);
  string *const str_ptr = &str;
  int chr = getc(stream);
  while (chr != EOF) {
    string_append_char(str_ptr, (char)chr);
    chr = getc(stream);
  }
  return str;
}

string string_new_stream_delim(FILE *const stream, const char delim) {
  string str = string_init(DEFAULT_CAPACITY);
  string *const str_ptr = &str;
  int chr = getc(stream);
  while (chr != delim && chr != EOF) {
    string_append_char(str_ptr, (char)chr);
    chr = getc(stream);
  }
  return str;
}

bool string_resize(string *const str, size_t new_capacity) {
  if (new_capacity != str->capacity) {
    void *const new_data = realloc(str->data, new_capacity + 1);
    if (new_data == NULL) return false;
    str->data = new_data;
    str->capacity = new_capacity;
    return true;
  }
  return false;
}

bool string_shrink_alloc(string *const str) {
  return string_resize(str, str->length);
}
