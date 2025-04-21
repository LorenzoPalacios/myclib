#ifndef STR_H
#define STR_H

#include <stdio.h>

#include "../include/boolmyclib.h"
#include "../include/compat.h"

/* - DEFINITIONS - */

typedef char *string;

typedef const char *const_string;

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199901L)
/* Widest basic integral type. */
typedef long long wb_int;
/* Widest basic unsigned integral type. */
typedef unsigned long long wb_uint;
#else
/* Widest basic integral type. */
typedef long wb_int;
/* Widest basic unsigned integral type. */
typedef unsigned long wb_uint;
#endif

#define STR_DEFAULT_CAPACITY (8192)

/* - CONVENIENCE MACROS - */

#define string_append_char(dst, src) string_append_char_(&(dst), src)

#define string_append_str(dst, src) string_append_str_(&(dst), src)

#define string_append_raw_str(dst, src) string_append_raw_str_(&(dst), src)

#define string_append_int(dst, num) string_append_int_(&(dst), (wb_int)(num))

#define string_append_uint(dst, num) string_append_uint_(&(dst), (wb_uint)(num))

#define string_delete(str) string_delete_(&(str))

#define string_expand(str) string_expand_(&(str))

#define string_expand_towards(str, target_capacity) \
  string_expand_towards_(&(str), target_capacity)

#define string_find_replace_str(src, tgt, repl) \
  string_find_replace_str_(&(src), tgt, repl)

#define string_find_replace_raw_str(src, tgt, repl) \
  string_find_replace_raw_str_(&(src), tgt, repl)

#define string_find_replace_char(src, tgt, repl) \
  string_find_replace_char_(&(src), tgt, repl)

#define string_insert_char(dst, chr, index) \
  string_insert_char_(&(dst), chr, index)

#define string_insert_int(dst, num, index) \
  string_insert_int_(&(dst), (wb_int)(num), index)

#define string_insert_uint(dst, num, index) \
  string_insert_uint_(&(dst), (wb_uint)(num), index)

#define string_insert_raw_str(dst, src, index) \
  string_insert_raw_str_(&(dst), src, index)

#define string_insert_str(dst, src, index) \
  string_insert_str_(&(dst), src, index)

#define string_resize(str, new_capacity) string_resize_(&(str), new_capacity)

#define string_shrink(str) string_shrink_(&(str))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L)

#define string_append(str, appended)           \
  (_Generic(appended,                          \
       short: string_append_int,               \
       int: string_append_int,                 \
       long: string_append_int,                \
       long long: string_append_int,           \
       unsigned short: string_append_uint,     \
       unsigned int: string_append_uint,       \
       unsigned long: string_append_uint,      \
       unsigned long long: string_append_uint, \
       string: string_append_raw_str,          \
       const_string: string_append_raw_str,    \
       char: string_append_char,               \
       signed char: string_append_char,        \
       unsigned char: string_append_char)(str, appended))

#define string_find_replace(src, needle, replacer)         \
  (_Generic(needle,                                        \
       string *: string_find_replace_str,                  \
       const string *: string_find_replace_str,            \
       char *: string_find_replace_raw_str,                \
       signed char *: string_find_replace_raw_str,         \
       unsigned char *: string_find_replace_raw_str,       \
       const char *: string_find_replace_raw_str,          \
       const signed char *: string_find_replace_raw_str,   \
       const unsigned char *: string_find_replace_raw_str, \
       char: string_find_replace_char,                     \
       signed char: string_find_replace_char,              \
       unsigned char: string_find_replace_char)(str, needle, replacer))

#define string_insert(str, inserted, index)    \
  (_Generic(inserted,                          \
       short: string_insert_int,               \
       int: string_insert_int,                 \
       long: string_insert_int,                \
       long long: string_insert_int,           \
       unsigned short: string_insert_uint,     \
       unsigned int: string_insert_uint,       \
       unsigned long: string_insert_uint,      \
       unsigned long long: string_insert_uint, \
       const_string: string_insert_str,        \
       string: string_insert_raw_str,          \
       char: string_insert_char,               \
       signed char: string_insert_char,        \
       unsigned char: string_insert_char)(str, inserted, index))

#if (__STDC_VERSION__ >= 202311L)
#define string(...)                                                 \
  (_Generic((__VA_OPT__((void)) STR_DEFAULT_CAPACITY __VA_OPT__(, ) \
                 __VA_ARGS__),                                      \
       char: string_from_char,                                      \
       signed char: string_from_char,                               \
       unsigned char: string_from_char,                             \
       short: string_init,                                          \
       int: string_init,                                            \
       long: string_init,                                           \
       long long: string_init,                                      \
       unsigned short: string_init,                                 \
       unsigned int: string_init,                                   \
       unsigned long: string_init,                                  \
       unsigned long long: string_init,                             \
       char *: string_from_raw_str,                                 \
       const char *: string_from_raw_str,                           \
       FILE *: string_from_stream)(                                 \
      (__VA_OPT__((void)) STR_DEFAULT_CAPACITY __VA_OPT__(, ) __VA_ARGS__)))
#else
#define string(arg)                       \
  (_Generic(arg,                          \
       char: string_from_char,            \
       signed char: string_from_char,     \
       unsigned char: string_from_char,   \
       short: string_init,                \
       int: string_init,                  \
       long: string_init,                 \
       long long: string_init,            \
       unsigned short: string_init,       \
       unsigned int: string_init,         \
       unsigned long: string_init,        \
       unsigned long long: string_init,   \
       char *: string_from_raw_str,       \
       const char *: string_from_raw_str, \
       FILE *: string_from_stream)(arg))
#endif

#endif

/* - FUNCTIONS - */

/**
 * @brief Appends a character to the end of the string.
 *
 * This function appends the given character to the end of the string,
 * expanding the string if necessary.
 *
 * @param dst A pointer to the string to which the character will be appended.
 * @param chr The character to append to the string.
 * @return `true` if the character was successfully appended. `false` otherwise.
 */
bool string_append_char_(string *dst, char chr);

/**
 * @brief Appends one string to another.
 *
 * This function appends the contents of the source string to the destination
 * string, expanding the destination string if necessary.
 *
 * @param dst A pointer to the destination string.
 * @param src A pointer to the source string.
 * @return `true` if the string was successfully appended. `false` otherwise.
 */
bool string_append_str_(string *dst, const_string src);

/**
 * @brief Appends a raw C-string to the end of the string.
 *
 * This function appends the given raw C-string to the end of the string,
 * expanding the string if necessary.
 *
 * @param dst A pointer to the string to which the raw C-string will be
 * appended.
 * @param src The raw C-string to append to the string.
 * @return `true` if the C-string was successfully appended. `false` otherwise.
 */
bool string_append_raw_str_(string *dst, const char *src);

/**
 * @brief Appends an integer to a string.
 *
 * This function converts the given integer to its string representation and
 * appends it to the end of the string, expanding the string if necessary.
 *
 * @param str A pointer to the string to which the integer will be appended.
 * @param num The integer to append to the string.
 * @return `true` if the integer was successfully appended. `false` otherwise.
 */
bool string_append_int_(string *str, wb_int num);

/**
 * @brief Appends an unsigned integer to a string.
 *
 * This function converts the given unsigned integer to its string
 * representation and appends it to the end of the string, expanding the string
 * if necessary.
 *
 * @param str A pointer to the string to which the integer will be appended.
 * @param num The `unsigned long long` integer to append to the string.
 * @return `true` if the unsigned integer was successfully appended. `false`
 * otherwise.
 */
bool string_append_uint_(string *str, wb_uint num);

/**
 * @brief Inserts an integer into a string at the specified index.
 *
 * This function converts the given integer to its string representation and
 * inserts it into the string at the specified index, expanding the string if
 * necessary.
 *
 * @param dst A pointer to the string.
 * @param num The integer to insert.
 * @param index The index at which to insert the integer.
 * @return `true` if the insertion occurred successfully. `false` otherwise.
 */
bool string_insert_int_(string *dst, wb_int num, size_t index);

/**
 * @brief Inserts an unsigned integer into a string at the specified index.
 *
 * This function converts the given integer to its string representation and
 * inserts it into the string at the specified index, expanding the string if
 * necessary.
 *
 * @param dst A pointer to the string.
 * @param num The unsigned integer to insert.
 * @param index The index at which to insert the integer.
 * @return `true` if the insertion occurred successfully. `false` otherwise.
 */
bool string_insert_uint_(string *dst, wb_uint num, size_t index);

size_t string_capacity(const_string str);

/**
 * @brief Clears the contents of the string.
 *
 * This function sets the length of the string to 0 and sets the first
 * character of the string data to a null terminator.
 *
 * @param str A pointer to the string to be cleared.
 */
void string_clear(string str);

string string_copy(const_string str);

void string_delete_(string *str);

bool string_equals(const_string str1, const_string str2);

/**
 * @brief Expands the capacity of the string.
 *
 * This function expands the capacity of the string by a predefined expansion
 * factor, updating the string's stats as necessary.
 *
 * @param str A pointer to the string to be expanded.
 * @return `true` if the expanion was successful. `false` otherwise.
 */
bool string_expand_(string *str);

/**
 * @brief Expands the capacity of the string to at least the target capacity.
 *
 * @param str A pointer to the string to be expanded.
 * @return `true` if the expanion was successful. `false` otherwise.
 */
bool string_expand_towards_(string *str, size_t target_capacity);

/**
 * @brief Finds and replaces the first occurrence of a substring within a
 * string.
 *
 * @param src A pointer to the haystack string.
 * @param tgt A pointer to the needle string.
 * @param replace A pointer to the replacement string.
 * @return `true` if the replacement occurred successfully. `false` otherwise.
 */
bool string_find_replace_str_(string *src, const_string tgt,
                              const_string replace);

/**
 * @brief Finds and replaces the first occurrence of a C-string.
 *
 * @param src A pointer to the haystack string.
 * @param tgt A pointer to the needle raw C-string.
 * @param replace A pointer to the replacement string.
 * @return `true` if the replacement occurred successfully. `false` otherwise.
 */
bool string_find_replace_raw_str_(string *src, const char *tgt,
                                  const_string replace);

/**
 * @brief Finds and replaces the first occurrence of a character within a
 * string.
 *
 * @param src A pointer to the haystack string.
 * @param tgt The needle character.
 * @param replace A pointer to the replacement string.
 * @return `true` if the replacement occurred successfully. `false` otherwise.
 */
bool string_find_replace_char_(string *src, char tgt, const_string replace);

/**
 * @brief Inserts a character into the string at the specified index.
 *
 * This function inserts the given character into the string at the specified
 * index, expanding the string if necessary.
 *
 * @param dst A pointer to the string.
 * @param chr The character to insert.
 * @param index The index at which to insert the character.
 * @return `true` if the insertion occurred successfully. `false` otherwise.
 */
bool string_insert_char_(string *dst, char chr, size_t index);

/**
 * @brief Inserts a raw C-string into a string at the specified index.
 *
 * This function inserts the given raw C-string into the string at the
 * specified index, expanding the string if necessary.
 *
 * @param dst A pointer to the string.
 * @param src The raw C-string to insert.
 * @param index The index at which to insert the raw C-string.
 * @return `true` if the insertion occurred successfully. `false` otherwise.
 */
bool string_insert_raw_str_(string *dst, const char *src, size_t index);

/**
 * @brief Inserts one string into another at the specified index.
 *
 * This function inserts the contents of the source string into the
 * destination string at the specified index, expanding the destination string
 * if necessary.
 *
 * @param dst A pointer to the destination string.
 * @param src A pointer to the source string.
 * @param index The index at which to insert the source string.
 * @return `true` if the insertion occurred successfully. `false` otherwise.
 */
bool string_insert_str_(string *dst, const_string src, size_t index);

/**
 * @brief Creates a new string of a specified capacity.
 *
 * This function creates a new string object capable of storing the specified
 * number of characters.
 *
 * @param capacity The number of characters the string should be able to store,
 * not including a null terminator.
 * @return A string capable of containing `capacity` characters.
 */
string string_init(size_t capacity);

/**
 * @brief Creates a new string containing a single character.
 *
 * This function creates a new string object containing the specified character.
 *
 * @param chr The character to be contained in the new string.
 * @return A string of default capacity whose contents are solely the passed
 * character.
 */
string string_from_char(char chr);

/**
 * @brief Creates a new string from a raw C-string.
 *
 * This function creates a new string object containing the characters from
 * the specified raw C-string.
 *
 * @param raw_str The raw C-string to be contained in the new string.
 * @return A string of default capacity whose contents are a copy of the passed
 * C-string.
 */
string string_from_raw_str(const char *raw_str);

/**
 * @brief Creates a new string from a single line of input from stdin.
 *
 * This function creates a new string object containing a single line of
 * characters read from stdin.
 *
 * @return A string whose contents are that of a single line of input from
 * `stdin`.
 */
string string_from_stdin(void);

/**
 * @brief Creates a new string from a stream.
 *
 * This function creates a new string object containing the characters read
 * from the specified stream until EOF is met.
 *
 * @param stream The stream to read characters from.
 * @return A string whose contents are all of the characters within the passed
 * stream.
 */
string string_from_stream(FILE *stream);

/**
 * @brief Creates a new string from a stream until a delimiter is met.
 *
 * This function creates a new string object containing the characters read
 * from the specified stream until either EOF or the specified delimiter
 * character is met.
 *
 * @param stream The stream to read characters from.
 * @param delim The delimiter character to stop reading at.
 * @return A string whose contents are all of the characters stored within the
 * passed stream up to the delimiter character. If the stream does not contain
 * the delimiter character, the contents of the string will be all of the
 * characters within the passed stream.
 */
string string_from_stream_delim(FILE *stream, char delim);

size_t string_length(const_string str);

/**
 * @brief Resizes the string to the specified number of characters.
 *
 * This function reallocates the memory used for the string to fit the
 * specified number of characters, updating the string's stats as necessary.
 *
 * @param str A pointer to the string to be resized.
 * @param new_capacity The new capacity of the string in bytes.
 * @return `true` if the string was successfully resized. `false` otherwise.
 */
bool string_resize_(string *str, size_t new_capacity);

/**
 * @brief Shrinks the memory used for the string to fit its contents.
 *
 * This function reallocates the memory used for the string to fit the number
 * of characters it contains.
 *
 * @param str A pointer to the string to be shrank.
 * @return `true` if the string was successfully shrank. `false` otherwise.
 */
bool string_shrink_(string *str);

#endif
