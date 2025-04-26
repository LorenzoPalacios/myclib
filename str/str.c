#include "str.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/myclib.h"

/* - DEFINITIONS - */

#define const_string_header(str) ((const string_header *)(str) - 1)

#define EXPANSION_FACTOR (2)

/*
 * `length`   - The number of characters present within the string (disregarding
 *              the null terminator).
 * `capacity` - The number of characters the string can store (disregarding the
 *              null terminator).
 */
typedef struct {
  size_t length;
  size_t capacity;
} string_header;

#define string_header(str) ((string_header *)(str) - 1)

/* - INTERNAL - */

static inline bool append_(string *const dst, const char *const src,
                           const size_t src_len) {
  const size_t OLD_LEN = string_length(*dst);
  const size_t NEW_LEN = OLD_LEN + src_len;
  if (string_capacity(*dst) < NEW_LEN)
    if (!string_expand_towards_(dst, NEW_LEN)) return false;
  memcpy(*dst + OLD_LEN, src, src_len);
  string_header(*dst)->length = NEW_LEN;
  (*dst)[NEW_LEN] = '\0';
  return true;
}

#define cnt_digits(n, b) \
  (cnt_digits_(((n) < 0 ? WB_UINT_MAX - (wb_uint)(n) + 1 : (wb_uint)(n)), b))

static inline size_t cnt_digits_(wb_uint n, const size_t base) {
  size_t digits = 1;
  if (n > base - 1)
    while (n /= base) digits++;
  return digits;
}

static bool find_replace_(string *const src, const char *const tgt,
                          const size_t tgt_len, const char *const repl,
                          const size_t repl_len) {
  const size_t SRC_LEN = string_length(*src);
  if (tgt_len <= SRC_LEN && tgt_len != 0) {
    char *const pos = (tgt_len == 1) ? strchr(*src, *tgt) : strstr(*src, tgt);
    if (pos != NULL) {
      const size_t POS_INDEX = (size_t)(pos - *src);
      const size_t NEW_LEN = SRC_LEN + repl_len - tgt_len;
      const size_t REMAINDER = SRC_LEN - POS_INDEX;
      if (string_capacity(*src) < NEW_LEN)
        if (!string_expand_towards_(src, NEW_LEN)) return false;
      memmove(*src + POS_INDEX, *src + POS_INDEX + tgt_len, REMAINDER);
      memmove(*src + POS_INDEX, repl, repl_len);
      string_header(*src)->length = NEW_LEN;
      (*src)[NEW_LEN] = '\0';
      return true;
    }
  }
  return false;
}

static bool insert_(string *const dst, const size_t index,
                    const char *const src, const size_t src_len) {
  const size_t NEW_LEN = string_length(*dst) + src_len;
  char *const dst_pos = *dst + index;
  const size_t REMAINDER = string_length(*dst) - index;
  if (index >= string_length(*dst)) return false;
  if (string_capacity(*dst) < NEW_LEN)
    if (!string_expand_towards_(dst, NEW_LEN)) return false;
  memmove(dst_pos + src_len, dst_pos, REMAINDER);
  memmove(dst_pos, src, src_len);
  string_header(*dst)->length = NEW_LEN;
  (*dst)[string_length(*dst)] = '\0';

  return true;
}

/* - LIBRARY FUNCTIONS - */

bool string_append_char_(string *const dst, const char chr) {
  return append_(dst, &chr, 1);
}

bool string_append_int_(string *const str, const wb_int num) {
  const size_t DIGIT_CNT = cnt_digits(num, 10);
  const size_t NEW_LEN = string_length(*str) + DIGIT_CNT;
  if (string_capacity(*str) < NEW_LEN)
    if (!string_expand_towards_(str, NEW_LEN)) return false;
  (void)sprintf(*str + string_length(*str), WB_INT_FMT, num);
  string_header(*str)->length = NEW_LEN;
  return true;
}

bool string_append_raw_str_(string *const dst, const char *src) {
  return append_(dst, src, strlen(src));
}

bool string_append_str_(string *const dst, const_string src) {
  return append_(dst, src, string_length(src));
}

bool string_append_uint_(string *const str, const wb_uint num) {
  const size_t DIGIT_CNT = cnt_digits(num, 10);
  const size_t NEW_LEN = string_length(*str) + DIGIT_CNT;
  if (string_capacity(*str) < NEW_LEN)
    if (!string_expand_towards_(str, NEW_LEN)) return false;
  (void)sprintf(*str + string_length(*str), WB_UINT_FMT, num);
  string_header(*str)->length = NEW_LEN;
  return true;
}

inline size_t string_capacity(const_string str) {
  return const_string_header(str)->capacity;
}

inline void string_clear(string str) {
  string_header(str)->length = 0;
  str[0] = '\0';
}

inline string string_copy(const_string str) {
  string copy = string_init(string_capacity(str));
  if (copy == NULL) return NULL;
  strcpy(copy, str);
  string_header(copy)->length = string_length(str);
  return copy;
}

inline void string_delete_(string *const str) {
  free(string_header(*str));
  *str = NULL;
}

inline bool string_equals(register const char *str1,
                          register const char *str2) {
  while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2)
    (void)(str1++), str2++;
  return *str1 == *str2;
}

inline bool string_expand_(string *const str) {
  const size_t NEW_CAPACITY =
      string_capacity(*str) == 0 ? 1 : EXPANSION_FACTOR * string_capacity(*str);
  return string_resize_(str, NEW_CAPACITY) ||
         string_resize_(str, string_capacity(*str) + 1);
}

inline bool string_expand_towards_(string *const str,
                                   const size_t target_capacity) {
  const size_t CUR_CAPACITY = string_capacity(*str);
  const size_t CAP_RATIO = (target_capacity / (CUR_CAPACITY + 1)) + 1;
  const size_t RESIZE_FACTOR = CAP_RATIO + (CAP_RATIO % EXPANSION_FACTOR);
  const size_t NEW_CAPACITY =
      RESIZE_FACTOR * (CUR_CAPACITY == 0 ? 1 : CUR_CAPACITY);
  return string_resize_(str, NEW_CAPACITY) ||
         string_resize_(str, target_capacity);
}

bool string_find_replace_str_(string *const src, const_string tgt,
                              const_string repl) {
  return find_replace_(src, tgt, string_length(tgt), repl, string_length(repl));
}

bool string_find_replace_raw_str_(string *const src, const char *const tgt,
                                  const_string repl) {
  const size_t TGT_LEN = strlen(tgt);
  return find_replace_(src, tgt, TGT_LEN, repl, string_length(repl));
}

bool string_find_replace_char_(string *const src, const char tgt,
                               const_string repl) {
  return find_replace_(src, &tgt, 1, repl, string_length(repl));
}

inline string string_from_char(const char chr) {
  string str = string_init(1);
  str[0] = chr;
  str[1] = '\0';
  string_header(str)->length = 1;
  return str;
}

inline string string_from_stdin(void) {
  return string_from_stream_delim(stdin, '\n');
}

string string_from_raw_str(const char *const raw_str) {
  string str = string_init(STR_DEFAULT_CAPACITY);
  size_t i;
  for (i = 0; (str[i] = raw_str[i]) != '\0'; i++) {
    if (i == string_capacity(str) && !string_expand_(&str)) {
      string_delete_(&str);
      return NULL;
    }
  }
  string_header(str)->length = i;
  return str;
}

string string_from_stream(FILE *const stream) {
  string str = string_init(STR_DEFAULT_CAPACITY);
  size_t i;
  int chr;
  for (i = 0; (chr = getc(stream)) != EOF; i++) {
    str[i] = INTEGRAL_CAST(chr, char);
    if (i == string_capacity(str) && !string_expand_(&str)) {
      string_delete_(&str);
      return NULL;
    }
  }
  string_header(str)->length = i;
  str[i] = '\0';
  return str;
}

string string_from_stream_delim(FILE *const stream, const char delim) {
  string str = string_init(STR_DEFAULT_CAPACITY);
  size_t i;
  int chr;
  for (i = 0; (chr = getc(stream)) != delim && chr != EOF; i++) {
    str[i] = INTEGRAL_CAST(chr, char);
    if (i == string_capacity(str) && !string_expand_(&str)) {
      string_delete_(&str);
      return NULL;
    }
  }
  string_header(str)->length = i;
  str[i] = '\0';
  return str;
}

inline string string_init(const size_t capacity) {
  const size_t ALLOCATION = capacity + sizeof(string_header) + 1;
  string str = (string)(((string_header *)malloc(ALLOCATION)) + 1);
  if (str == NULL) return NULL;
  str[0] = '\0';
  string_header(str)->length = 0;
  string_header(str)->capacity = capacity;
  return str;
}

bool string_insert_char_(string *const dst, const char chr,
                         const size_t index) {
  return insert_(dst, index, &chr, 1);
}

bool string_insert_int_(string *const str, const wb_int num,
                        const size_t index) {
  const size_t DIGIT_CNT = cnt_digits(num, 10) + (num < 0 ? 1 : 0);
  const size_t NEW_LEN = string_length(*str) + DIGIT_CNT;
  const char OVERWRITTEN_CHR = (*str)[index];
  if (string_capacity(*str) < NEW_LEN)
    if (!string_expand_towards_(str, NEW_LEN)) return false;
  memmove(*str + index + DIGIT_CNT, *str + index, string_length(*str) - index);
  (void)sprintf(*str + index, WB_INT_FMT, num);
  (*str)[index + DIGIT_CNT] = OVERWRITTEN_CHR;
  string_header(*str)->length = NEW_LEN;
  (*str)[NEW_LEN] = '\0';
  return true;
}

bool string_insert_raw_str_(string *dst, const char *const src,
                            const size_t index) {
  const size_t SRC_LEN = strlen(src);
  return insert_(dst, index, src, SRC_LEN);
}

bool string_insert_str_(string *const dst, const_string src,
                        const size_t index) {
  return insert_(dst, index, src, string_length(src));
}

bool string_insert_uint_(string *const str, const wb_uint num,
                         const size_t index) {
  const size_t DIGIT_CNT = cnt_digits(num, 10);
  const size_t NEW_LEN = string_length(*str) + DIGIT_CNT;
  const char OVERWRITTEN_CHR = (*str)[index];
  if (string_capacity(*str) < NEW_LEN)
    if (!string_expand_towards_(str, NEW_LEN)) return false;
  memmove(*str + index + DIGIT_CNT, *str + index, string_length(*str) - index);
  (void)sprintf(*str + index, WB_UINT_FMT, num);
  (*str)[index + DIGIT_CNT] = OVERWRITTEN_CHR;
  string_header(*str)->length = NEW_LEN;
  (*str)[NEW_LEN] = '\0';
  return true;
}

inline size_t string_length(const_string str) {
  return const_string_header(str)->length;
}

inline bool string_resize_(string *const str, const size_t new_capacity) {
  /* Adding one to account for a null terminator. */
  const size_t ALLOCATION = sizeof(string_header) + new_capacity + 1;
  string new_str =
      (string)(((string_header *)realloc(string_header(*str), ALLOCATION)) + 1);
  if (new_str == NULL) return false;
  string_header(new_str)->capacity = new_capacity;
  if (new_capacity < string_length(new_str)) {
    string_header(new_str)->length = new_capacity;
    new_str[new_capacity] = '\0';
  }
  *str = new_str;
  return true;
}

inline bool string_shrink_(string *const str) {
  return string_resize_(str, string_length(*str));
}
