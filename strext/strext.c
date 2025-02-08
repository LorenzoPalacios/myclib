#include "strext.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPANSION_FACTOR (2)
#define BASE_STR_CAPACITY (4096 - sizeof(string) - 1)

// - INTERNAL -

// Used by constructors and reallocators to calculate how much memory should be
// allocated for a string.
static inline size_t calc_allocation(const size_t requested_cap) {
  return requested_cap + sizeof(string) + 1;
}

size_t string_capacity(const string *const str) {
  // Subtracting one since a null terminator must be present in the string.
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
  const size_t DST_CAPACITY = string_capacity(str);
  const size_t EXPANSION_CNT = new_capacity / DST_CAPACITY;
  if (EXPANSION_CNT != 0) {
    const size_t EXPANDED_CAPACITY =
        EXPANSION_FACTOR * EXPANSION_CNT * DST_CAPACITY;
    string *expanded_str = string_resize(str, EXPANDED_CAPACITY);
    if (expanded_str == NULL) expanded_str = string_resize(str, new_capacity);
    str = expanded_str;
  }
  return str;
}

static string *find_replace_(string *src, const char *const tgt,
                             const size_t tgt_len, const char *const repl,
                             const size_t repl_len) {
  if (tgt_len == 0 || tgt_len > src->length) return src;

  char *const needle_pos = strstr(src->data, tgt);
  if (needle_pos != NULL) {
    const size_t REQ_CAPACITY = src->length + repl_len - tgt_len;
    string *const realloc_str = expand_str_to_capacity(src, REQ_CAPACITY);
    if (realloc_str == NULL) return NULL;
    src = realloc_str;

    const size_t REMAINDER = src->length - (size_t)(needle_pos - src->data);
    memmove(needle_pos + repl_len, needle_pos + tgt_len, REMAINDER);
    // Using `memcpy()` since `strcpy()` appends a null terminator.
    memcpy(needle_pos, repl, repl_len);
  }
  src->length += repl_len;
  src->length -= tgt_len;
  src->data[src->length] = '\0';
  return src;
}

static string *string_insert_(string *dst, size_t index, const char *const src,
                              size_t src_len) {
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

// - LIBRARY FUNCTIONS -

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
  const size_t DIGIT_CNT = num == 0 ? 1 : (size_t)log10l((long double)num) + 1;
  const size_t REQ_CAPACITY = str->length + DIGIT_CNT;
  if (string_capacity(str) < REQ_CAPACITY) {
    str = expand_str_to_capacity(str, REQ_CAPACITY);
    if (str == NULL) return NULL;
  }
  (void)sprintf(str->data + str->length, "%llu", num);
  str->length += DIGIT_CNT;
  return str;
}

void string_clear(string *const str) {
  str->length = 0;
  str->data[str->length] = '\0';
}

void string_delete(string *const str) { free(str); }

string *string_expand(string *const str) {
  const size_t CUR_CAP = string_capacity(str);
  const size_t NEW_CAP = (CUR_CAP > 1) ? EXPANSION_FACTOR * CUR_CAP : 1;
  return string_resize(str, NEW_CAP);
}

string *string_find_replace_str(string *const src, const string *const tgt,
                                const string *const repl) {
  return find_replace_(src, tgt->data, tgt->length, repl->data, repl->length);
}

string *string_find_replace_raw_str(string *const src, const char *const tgt,
                                    const string *const repl) {
  const size_t TGT_LEN = strlen(tgt);
  return find_replace_(src, tgt, TGT_LEN, repl->data, repl->length);
}

string *string_find_replace_char(string *const src, const char tgt,
                                 const string *const repl) {
  return find_replace_(src, &tgt, 1, repl->data, repl->length);
}

string *string_insert_char(string *const dst, const char chr,
                           const size_t index) {
  return string_insert_(dst, index, &chr, 1);
}

string *string_insert_raw_str(string *dst, const char *const src,
                              const size_t index) {
  const size_t SRC_LEN = strlen(src);
  return string_insert_(dst, index, src, SRC_LEN);
}

string *string_insert_str(string *const dst, const string *const src,
                          const size_t index) {
  return string_insert_(dst, index, src->data, src->length);
}

string *string_of_capacity(const size_t capacity) {
  const size_t ALLOCATION = calc_allocation(capacity);
  string *const str = malloc(ALLOCATION);
  if (str == NULL) return NULL;
  str->data = get_string_contents(str);
  str->length = 0;
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

string *string_init(void) { return string_of_capacity(BASE_STR_CAPACITY); }

string *string_of_raw_str(const char *raw_str) {
  string *str = string_init();
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
  string *str = string_init();

  int chr = getc(stream);
  while (chr != EOF) {
    string *const realloc_str = string_append(str, (char)chr);
    if (realloc_str == NULL) {
      string_delete(str);
      return NULL;
    }
    str = realloc_str;
    chr = getc(stream);
  }
  return str;
}

string *string_of_stream_delim(FILE *const stream, const char delim) {
  string *str = string_init();

  int chr = getc(stream);
  while (chr != delim && chr != EOF) {
    string *const realloc_str = string_append(str, (char)chr);
    if (realloc_str == NULL) {
      string_delete(str);
      return NULL;
    }
    str = realloc_str;
    chr = getc(stream);
  }
  return str;
}

string *string_resize(string *str, const size_t new_capacity) {
  const size_t ALLOCATION = calc_allocation(new_capacity);
  {
    string *const new_str = realloc(str, ALLOCATION);
    if (new_str == NULL) return NULL;
    str = new_str;
  }
  str->data = get_string_contents(str);
  str->allocation = ALLOCATION;

  if (new_capacity < str->length) {
    str->length = new_capacity;
    str->data[str->length] = '\0';
  }
  return str;
}

string *string_shrink_alloc(string *const str) {
  return string_resize(str, str->length);
}
