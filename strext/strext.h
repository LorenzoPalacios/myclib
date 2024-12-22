#ifndef _STR_EXT
#define _STR_EXT

#include <stdio.h>

typedef struct {
  char *data;
  size_t length;
  size_t capacity;
  size_t allocation;
} string;

#define new_string(arg)                          \
  (_Generic((arg),                               \
       char: string_of_char,                     \
       signed char: string_of_char,              \
       unsigned char: string_of_char,            \
       short: string_of_capacity,                \
       int: string_of_capacity,                  \
       long: string_of_capacity,                 \
       long long: string_of_capacity,            \
       unsigned short: string_of_capacity,       \
       unsigned int: string_of_capacity,         \
       unsigned long: string_of_capacity,        \
       unsigned long long: string_of_capacity,   \
       char *: string_of_raw_str,                \
       signed char *: string_of_raw_str,         \
       unsigned char *: string_of_raw_str,       \
       const char *: string_of_raw_str,          \
       const signed char *: string_of_raw_str,   \
       const unsigned char *: string_of_raw_str, \
       FILE *: string_of_stream)(arg))

#define string_append(appended)             \
  (_Generic((arg),                          \
       const char *: string_append_raw_str, \
       char *: string_append_raw_str,       \
       signed char: string_append_char,     \
       unsigned char: string_append_char)(arg))

#define delete_string(str) _delete_string(&(str))
#define delete_string_s(str) _delete_string_s(&(str))

/*
 * Appends `appended` to the end of `dst`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if
 * the operation failed.
 */
string *string_append_char(string *dst, char appended);

/*
 * Appends `src` to the end of `dst_str`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if the
 * operation failed.
 */
string *string_append_str(string *dst, string *src);

/*
 * Appends `src` to the end of `dst_str`, expanding if necessary.
 *
 * \return A pointer associated with the data of `dst`, or `NULL` if the
 * operation failed.
 */
string *append_raw_str(string *dst, const char *src, size_t src_len);

void _string_delete(string **str_obj);

void _string_delete_s(string **str_obj);

string *string_clear(string *str);

string *string_clear_s(string *str);

/*
 * Expands the passed string's allocated boundaries by
 * `expansion_factor`, updating the stats of `str_obj` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of
 * `str_obj`, or `NULL` if reallocation failed.
 *
 * \note If reallocation failed, `str_obj` will be unmodified.
 */
string *string_expand(string *str_obj);

/*
 * Finds the first occurrence of `needle` within `haystack` starting
 * from the beginning of `haystack->data` and replaces it with
 * `replacement`, expanding `haystack` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of
 * `haystack`, or `NULL` if the operation failed.
 *
 * \note A returned `NULL` pointer does not guarantee `haystack` is
 * unmodified by this function.
 */
string *string_find_replace(string *hay, const string *needle,
                            const string *replace);

/*
 * Finds all occurrences of `needle` within `haystack` starting from the
 * beginning of `haystack->data`, replacing any findings with
 * `replacement` and expanding `haystack` as necessary.
 *
 * \return A (possibly new) pointer associated with the data of
 * `haystack`, or `NULL` if the operation failed.
 *
 * \note A returned `NULL` pointer does not guarantee `haystack` is
 * unmodified by this function.
 *
 * \attention Function not yet complete.
 */
string *string_find_replace_all(string *hay, const string *needle,
                                const string *replace);

/*
 * Reallocates the memory used for `str_obj` to fit `new_size` bytes,
 * updating the stats of `str_obj` as necessary.
 *
 * This function only modifies the memory allocated for characters,
 * meaning there will always be enough space for the data members of
 * `string` regardless of the value passed as `new_size`.
 *
 * \return A (possibly new) pointer associated with the data of
 * `str_obj`, or `NULL` if reallocation failed.
 *
 * \note If reallocation failed, `str_obj` will be unmodified.
 */
string *string_resize(string *str_obj, size_t new_size);

/*
 * Shrinks the memory used for `str_obj` to fit the number of characters
 * it contains.
 *
 * \return A (possibly new) pointer associated with the data of
 * `str_obj`, or `NULL` if reallocation failed.
 */
string *string_shrink_alloc(string *str_obj);

string *string_new(void);

string *string_of_char(char chr);

/*
 * Generates a `string` object whose `data` consists of the passed raw
 * null-terminated string, `raw_str`.
 *
 * \return A `string` object containing characters from `raw_str`, or
 * `NULL` upon failure.
 */
string *string_of_raw_str(const char *raw_str);

/*
 * Generates a `string` object whose `data` consists of a single line of
 * characters from `stdin`.
 *
 * \return A `string` object containing characters from `stdin`, or
 * `NULL` upon failure.
 */
string *string_of_line_stdin(void);

/*
 * Creates a `string` object whose `data` consists of characters within
 * `stream` until `EOF` is met.
 *
 * \return A pointer to a `string` object containing characters from
 * `stream`, or `NULL` upon failure.
 */
string *string_of_stream(FILE *stream);

/*
 * Creates a `string` object whose `data` consists of characters from
 * `stream` until either `EOF` or the specified `delim` character is
 * met.
 *
 * \return A pointer to a `string` object containing characters from
 * `stream`, or `NULL` upon failure.
 */
string *string_of_stream_delim(FILE *stream, char delim);

/*
 * Creates a `string` object capable of storing `capacity` number of
 * characters.
 *
 * \return A pointer to a `string` object of size
 * `capacity + sizeof(string)`, or `NULL` upon failure.
 */
string *string_of_capacity(const size_t capacity);

#endif
