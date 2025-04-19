#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/boolmyclib.h"
#include "../include/compat.h"
#include "../include/utilmyclib.h"

/* - DEFINITIONS - */

#define STR_DEFAULT_CAPACITY (8192)

typedef struct {
  size_t length;
  size_t capacity;
} string_header;

typedef char *string;

typedef const char *const_string;

/* - INTERNAL USE ONLY - */

#define STR_EXPANSION_FACTOR (2)

#define string_header(str) \
  ((string_header *)((unsigned char *)(str) - sizeof(string_header)))

#define string_header_const(str)                          \
  ((const string_header *)((const unsigned char *)(str) - \
                           sizeof(string_header)))

/* - MACROS - */

/* Returns the total amount of bytes allocated for a string. */
#define string_alloc_size(str) \
  (sizeof(string_header) + string_capacity(str) + 1)

#define string_append_chr(dst, chr)                         \
  (inline_if(string_capacity(dst) < string_length(dst) + 1, \
             string_expand(dst), NULL),                     \
   (dst)[string_length(dst)] = (chr),                       \
   (dst)[++string_header(dst)->length] = '\0', true)

#define string_append_chr_s(dst, chr) string_append_chr_f(&(dst), chr)

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)

#define string_append_int(dst, num)                       \
  inline_if((num) < 0,                                    \
            (string_append_chr(dst, '-'),                 \
             string_append_uint(dst, -(long long)(num))), \
            string_append_uint(dst, num))

#define string_append_uint(dst, num)                                         \
  ((void)(inline_if(                                                         \
              string_capacity(dst) < string_length(dst) + cnt_digits(num),   \
              string_expand_towards_f(dst,                                   \
                                      string_length(dst) + cnt_digits(num)), \
              NULL),                                                         \
          sprintf((dst), "%llu", (unsigned long long)(num)),                 \
          string_header(dst)->length += cnt_digits(num)),                    \
   true)

#endif

#define string_append_str(dst, src)                                          \
  (inline_if(                                                                \
       string_capacity(dst) < string_length(dst) + string_length(src),       \
       string_expand_towards(dst, string_length(dst) + string_length(src))), \
   strcpy((dst) + string_length(dst), src),                                  \
   string_header(dst)->length += string_length(src), true)

#define string_append_str_s(str, src) string_append_str_f(&(str), src)

#define string_capacity(str) (+string_header_const(str)->capacity)

#define string_copy(str)                                              \
  ((string)((string_header *)memcpy(                                  \
                malloc(sizeof(string_header) + string_capacity(str)), \
                string_header_const(str),                             \
                malloc(sizeof(string_header) + string_length(str))) + \
            1))

#define string_copy_s(str) string_copy_f(str)

#define string_delete(str) free(string_header(str))

#define string_expand(str)                                              \
  (string_resize(str, string_capacity(str) != 0                         \
                          ? STR_EXPANSION_FACTOR * string_capacity(str) \
                          : 1))

#define string_expand_towards(str, desired_capacity)                          \
  string_resize(str, ((target_capacity / (string_capacity(str) + 1)) +        \
                      ((target_capacity / (string_capacity(str) + 1)) %       \
                       STR_EXPANSION_FACTOR)) == 0                            \
                         ? 1                                                  \
                         : ((target_capacity / (string_capacity(str) + 1)) +  \
                            ((target_capacity / (string_capacity(str) + 1)) % \
                             STR_EXPANSION_FACTOR)) *                         \
                               string_capacity(str))

#define string_expand_s(str) string_expand_f(&(str))

#define string_length(str) (+string_header_const(str)->length)

#define string_insert_chr(dst, chr, index)                                   \
  ((void)(inline_if(string_capacity(dst) < string_length(dst) + 1,           \
                    string_expand(dst), NULL),                               \
          memmove((dst) + 1, (dst) + (index), string_length(dst) - (index)), \
          (dst)[index] = (chr), (dst)[++string_header(dst)->length] = '\0'), \
   true)

#define string_insert_int(dst, num, index)                                 \
  ((void)(inline_if(string_capacity(dst) < string_length(dst) +            \
                                               cnt_digits(num) +           \
                                               ((num) < 0 ? 1 : 0),        \
                    string_expand(dst), NULL),                             \
          memmove((dst) + (index) + cnt_digits(num), (dst) + (index),   \
                  string_length(dst) - (index)),                           \
          (dst)[string_length(dst)] =                                      \
              (dst)[cnt_digits(num) + ((num) < 0 ? 1 : 0)],                \
          sprintf((dst) + (index), "%lld", (long long)(num)),               \
          (dst)[cnt_digits(num) + ((num) < 0 ? 1 : 0)] =                   \
              (dst)[string_length(dst)],                                   \
          (dst)[string_header(dst)->length += cnt_digits(num)] = '\0'), \
   true)

#define string_insert_uint(dst, num, index)                                \
  ((void)(inline_if(                                                       \
              string_capacity(dst) < string_length(dst) + cnt_digits(num), \
              string_expand(dst), NULL),                                   \
          memmove((dst) + (index) + string_length(src), (dst) + (index),   \
                  string_length(dst) - (index)),                           \
          (dst)[string_length(dst)] = (dst)[cnt_digits(num)],              \
          sprintf((dst) + (index), src, string_length(src)),               \
          (dst)[cnt_digits(num)] = (dst)[string_length(dst)],              \
          (dst)[string_header(dst)->length += string_length(src)] = '\0'), \
   true)

#define string_insert_str(dst, src, index)                                    \
  ((void)(inline_if(                                                          \
              string_capacity(dst) < string_length(dst) + string_length(src), \
              string_expand(dst), NULL),                                      \
          memmove((dst) + (index) + string_length(src), (dst) + (index),      \
                  string_length(dst) - (index)),                              \
          memcpy((dst) + (index), src, string_length(src)),                   \
          (dst)[string_header(dst)->length += string_length(src)] = '\0'),    \
   true)

#define string_new() string_new_f(STR_DEFAULT_CAPACITY)

#define string_resize(str, new_capacity)                                       \
  ((void)(util_assert((new_capacity) >= 0),                                    \
          util_assert(                                                         \
              ((str) = (string)(((string_header *)realloc(                     \
                                    string_header(str),                        \
                                    sizeof(string_header) + (new_capacity))) + \
                                1)) != NULL),                                  \
          string_header(str)->capacity = (new_capacity) > string_length(str)   \
                                             ? string_header(str)->length =    \
                                                   (new_capacity)              \
                                             : (new_capacity)),                \
   (true))

#define string_shrink(str) string_resize(str, string_length(str))

/* - FUNCTION DECLARATIONS - */

static bool string_append_chr_f(string *dst, char chr);
static bool string_append_int_f(string *dst, long long num);
static bool string_append_uint_f(string *dst, unsigned long long num);

static void string_clear(string str);

static string string_copy_raw_str(const char *str, size_t str_len);
static string string_copy_f(const_string str);

static bool string_expand_f(string *str);
static bool string_expand_towards_f(string *str, size_t target_capacity);

static bool string_find_replace(string *src, const_string target,
                                const_string repl);

static bool string_insert_chr_f(string *dst, char chr, size_t index);
static bool string_insert_int_f(string *dst, long long num, size_t index);
static bool string_insert_uint_f(string *dst, unsigned long long num,
                                 size_t index);
static bool string_insert_str_f(string *dst, const_string src, size_t index);

static string string_new_f(size_t capacity);

static bool string_resize_f(string *str, size_t new_capacity);
static bool string_shrink_f(string *str);

/* - FUNCTIONS - */

static inline bool string_append_chr_f(string *const dst, const char chr) {
  return string_insert_chr_f(dst, chr, string_length(*dst));
}

static inline bool string_append_int_f(string *dst, long long num) {
  return string_insert_int_f(dst, num, string_length(*dst));
}
static inline bool string_append_uint_f(string *dst, unsigned long long num) {
  return string_insert_uint_f(dst, num, string_length(*dst));
}

static inline string string_copy_raw_str(const char *const str,
                                         const size_t str_len) {
  string copy = string_new_f(str_len);
  if (copy != NULL) {
    memcpy(copy, str, str_len);
    string_header(copy)->length = str_len;
  }
  return copy;
}

static inline string string_copy_f(const_string str) {
  void *copy = malloc(string_alloc_size(str));
  if (copy != NULL) {
    memcpy(copy, str, sizeof(string_header) + string_length(str));
    copy = (string_header *)copy + 1;
  }
  return copy;
}

static inline bool string_expand_f(string *const str) {
  const size_t NEW_CAPACITY =
      string_capacity(*str) == 0 ? 1
                                 : STR_EXPANSION_FACTOR * string_capacity(*str);
  return string_resize_f(str, NEW_CAPACITY) ||
         string_resize_f(str, string_capacity(str) + 1);
}

static inline bool string_expand_towards_f(string *const str,
                                           const size_t target_capacity) {
  const size_t CAP_RATIO = target_capacity / (string_capacity(*str) + 1);
  const size_t RESIZE_FACTOR = CAP_RATIO + (CAP_RATIO % STR_EXPANSION_FACTOR);
  const size_t NEW_CAPACITY =
      RESIZE_FACTOR == 0 ? 1 : RESIZE_FACTOR * string_capacity(*str);
  return string_resize_f(str, NEW_CAPACITY) ||
         string_resize_f(str, target_capacity);
}

static inline bool string_find_replace(string *const src, const_string target,
                                       const_string repl) {
  const size_t TGT_LEN = string_length(target);
  const size_t REPL_LEN = string_length(repl);
  if (string_length(*src) >= TGT_LEN && TGT_LEN != 0) {
    string pos = strstr(*src, target);
    if (pos != NULL) {
      const size_t POS_INDEX = (size_t)(pos - *src);
      const size_t NEW_LEN = string_length(*src) - TGT_LEN + REPL_LEN;
      const size_t DIFF = string_length(*src) - TGT_LEN - POS_INDEX;
      if (NEW_LEN > string_capacity(*src)) {
        if (!string_expand_towards_f(src, NEW_LEN)) return false;
        /* In case the original pointer is invalidated by resizing. */
        pos = *src + POS_INDEX;
      }
      memmove(pos + REPL_LEN, pos + TGT_LEN, DIFF);
      memcpy(pos, repl, REPL_LEN);
      string_header(*src)->length = NEW_LEN;
      (*src)[string_length(*src)] = '\0';
    }
  }
  return true;
}

static inline bool string_insert_chr_f(string *const dst, const char chr,
                                       const size_t index) {
  const size_t STR_LEN = string_length(*dst);
  if (string_capacity(*dst) < STR_LEN + 1)
    if (!string_expand_f(dst)) return false;
  memmove(*dst + index + 1, *dst + index, STR_LEN - index);
  (*dst)[index] = chr;
  (*dst)[STR_LEN + 1] = '\0';
  string_header(*dst)->length++;
  return true;
}

static inline bool string_insert_str_f(string *const dst, const_string src,
                                       const size_t index) {
  const size_t SRC_LEN = string_length(src);
  const size_t NEW_LEN = string_length(*dst) + SRC_LEN;
  string pos = *dst + index;
  if (string_capacity(dst) < NEW_LEN)
    if (!string_expand_towards_f(dst, NEW_LEN)) return false;
  memmove(pos + SRC_LEN, pos, string_length(*dst) - index);
  memcpy(pos, src, SRC_LEN);
  string_header(*dst)->length = NEW_LEN;
  return true;
}

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)

static inline bool string_insert_int_f(string *const dst, long long num,
                                       const size_t index) {
  if (num < 0) {
    string_insert_chr_f(dst, '-', index);
    num = -num;
  }
  return string_insert_uint_f(dst, (unsigned long long)num, index + 1);
}

static inline bool string_insert_uint_f(string *const dst,
                                        const unsigned long long num,
                                        const size_t index) {
  const size_t DIGIT_CNT = cnt_digits(num);
  const size_t REQ_CAPACITY = string_length(*dst) + DIGIT_CNT;
  string pos = *dst + index;
  const char ORIGINAL_CHAR = pos[DIGIT_CNT];
  if (string_capacity(*dst) < REQ_CAPACITY)
    if (!string_expand_towards_f(dst, REQ_CAPACITY)) return false;
  memmove(pos + DIGIT_CNT, pos, string_length(*dst) - index);
  sprintf(pos, "%llu", num);
  pos[DIGIT_CNT] = ORIGINAL_CHAR;
  return true;
}

#else

static inline bool string_insert_int_f(string *const dst, long num,
                                       const size_t index) {
  if (num < 0) {
    string_insert_chr_f(dst, '-', index);
    num = -num;
  }
  return string_insert_uint_f(dst, (unsigned long)num, index + 1);
}

static inline bool string_insert_uint_f(string *const dst,
                                        const unsigned long num,
                                        const size_t index) {
  const size_t DIGIT_CNT = cnt_digits(num);
  const size_t REQ_CAPACITY = string_length(*dst) + DIGIT_CNT;
  string pos = *dst + index;
  if (string_capacity(*dst) < REQ_CAPACITY)
    if (!string_expand_towards_f(dst, REQ_CAPACITY)) return false;
  memmove(pos + DIGIT_CNT, pos, string_length(*dst) - index);
  sprintf(pos, "%lu");
  return true;
}

#endif

static inline string string_new_f(const size_t capacity) {
  /* Adding one to account for a null terminator. */
  string_header *const str = malloc(sizeof(string_header) + capacity + 1);
  if (str == NULL) return NULL;
  str->capacity = capacity;
  str->length = 0;
  ((string)(str + 1))[capacity] = '\0';
  return (string)(str + 1);
}

static inline bool string_resize_f(string *const str,
                                   const size_t new_capacity) {
  /* Adding one to account for a null terminator. */
  const size_t ALLOCATION = sizeof(string_header) + new_capacity + 1;
  string new_str =
      (void *)(((string_header *)realloc(string_header(*str), ALLOCATION)) + 1);
  if (new_str == NULL) return false;
  string_header(new_str)->capacity = new_capacity;
  if (new_capacity < string_length(new_str)) {
    string_header(new_str)->length = new_capacity;
    new_str[new_capacity] = '\0';
  }
  *str = new_str;
  return true;
}

int main(void) {
  string a = string_new();
  string_insert_int(a, 202, 0);
  puts(a);
  return 0;
}

#endif
