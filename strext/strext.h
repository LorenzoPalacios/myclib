#ifndef _STR_EXT
#define _STR_EXT

#include <stdio.h>

typedef struct {
  char *data;
  size_t length;
  size_t capacity;
  size_t allocation;
} string;

#define string_new(arg)                          \
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

#define string_append(str, appended)                 \
  (_Generic((appended),                              \
       short: string_append_int,                     \
       int: string_append_int,                       \
       long: string_append_int,                      \
       long long: string_append_int,                 \
       unsigned short: string_append_uint,           \
       unsigned int: string_append_uint,             \
       unsigned long: string_append_uint,            \
       unsigned long long: string_append_uint,       \
       char *: string_append_raw_str,                \
       signed char *: string_append_raw_str,         \
       unsigned char *: string_append_raw_str,       \
       const char *: string_append_raw_str,          \
       const signed char *: string_append_raw_str,   \
       const unsigned char *: string_append_raw_str, \
       char: string_append_char,                     \
       signed char: string_append_char,              \
       unsigned char: string_append_char)(str, appended))

#define string_delete(str) _string_delete(&(str))
#define string_delete_s(str) _string_delete_s(&(str))

/**
 * @brief Appends a character to the end of the string.
 *
 * This function appends the given character to the end of the string,
 * expanding the string if necessary.
 *
 * @param dst A pointer to the string to which the character will be appended.
 * @param chr The character to append to the string.
 * @return A pointer to the modified string, or NULL if the operation failed.
 */
string *string_append_char(string *dst, char appended);

/**
 * @brief Appends one string to another.
 *
 * This function appends the contents of the source string to the destination
 * string, expanding the destination string if necessary.
 *
 * @param dst A pointer to the destination string.
 * @param src A pointer to the source string.
 * @return A pointer to the modified destination string, or NULL if the operation failed.
 */
string *string_append_str(string *dst, string *src);

/**
 * @brief Appends a raw C-string to the end of the string.
 *
 * This function appends the given raw C-string to the end of the string,
 * expanding the string if necessary.
 *
 * @param dst A pointer to the string to which the raw C-string will be appended.
 * @param src The raw C-string to append to the string.
 * @return A pointer to the modified string, or NULL if the operation failed.
 */
string *string_append_raw_str(string *dst, const char *src);

/**
 * @brief Appends a long long integer to the string.
 *
 * This function converts the given long long integer to its string
 * representation and appends it to the end of the string, expanding the
 * string if necessary.
 *
 * @param str A pointer to the string to which the integer will be appended.
 * @param num The long long integer to append to the string.
 * @return A pointer to the modified string, or NULL if the operation failed.
 */
string *string_append_int(string *str, long long num);

/**
 * @brief Appends an unsigned long long integer to the string.
 *
 * This function converts the given unsigned long long integer to its string
 * representation and appends it to the end of the string, expanding the
 * string if necessary.
 *
 * @param str A pointer to the string to which the integer will be appended.
 * @param num The unsigned long long integer to append to the string.
 * @return A pointer to the modified string, or NULL if the operation failed.
 */
string *string_append_uint(string *str, unsigned long long num);

/**
 * @brief Clears the contents of the string.
 *
 * This function sets the length of the string to 0 and sets the first
 * character of the string data to the null terminator.
 *
 * @param str A pointer to the string to be cleared.
 */
void string_clear(string *str);

/**
 * @brief Securely clears the contents of the string.
 *
 * This function sets all characters in the string data to the null terminator
 * and sets the length of the string to 0.
 *
 * @param str A pointer to the string to be securely cleared.
 */
void string_clear_s(string *str);

/**
 * @brief Deletes the string and frees its memory.
 *
 * This function frees the memory allocated for the string and sets the
 * string pointer to NULL.
 *
 * @param str A pointer to the string pointer to be deleted.
 */
void _string_delete(string **str);

/**
 * @brief Securely deletes the string and frees its memory.
 *
 * This function sets all bytes in the string to 0, frees the memory allocated
 * for the string, and sets the string pointer to NULL.
 *
 * @param str A pointer to the string pointer to be securely deleted.
 */
void _string_delete_s(string **str);

/**
 * @brief Expands the capacity of the string.
 *
 * This function expands the capacity of the string by a predefined expansion
 * factor, updating the string's stats as necessary.
 *
 * @param str A pointer to the string to be expanded.
 * @return A pointer to the expanded string, or NULL if the operation failed.
 */
string *string_expand(string *str);

/**
 * @brief Finds and replaces the first occurrence of a substring within a string.
 *
 * This function finds the first occurrence of the needle string within the
 * haystack string and replaces it with the replacement string, expanding the
 * haystack string if necessary.
 *
 * @param hay A pointer to the haystack string.
 * @param needle A pointer to the needle string.
 * @param replace A pointer to the replacement string.
 * @return A pointer to the modified haystack string, or NULL if the operation failed.
 */
string *string_find_replace(string *hay, const string *needle,
                            const string *replace);

/**
 * @brief Creates a new string with a default capacity.
 *
 * This function creates a new string object with a default capacity.
 *
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_new_default(void);

/**
 * @brief Creates a new string with a specified capacity.
 *
 * This function creates a new string object capable of storing the specified
 * number of characters.
 *
 * @param capacity The number of characters the string should be able to store.
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_capacity(size_t capacity);

/**
 * @brief Creates a new string containing a single character.
 *
 * This function creates a new string object containing the specified character.
 *
 * @param chr The character to be contained in the new string.
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_char(char chr);

/**
 * @brief Creates a new string from a raw C-string.
 *
 * This function creates a new string object containing the characters from
 * the specified raw C-string.
 *
 * @param raw_str The raw C-string to be contained in the new string.
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_raw_str(const char *raw_str);

/**
 * @brief Creates a new string from a single line of input from stdin.
 *
 * This function creates a new string object containing a single line of
 * characters read from stdin.
 *
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_line_stdin(void);

/**
 * @brief Creates a new string from a stream.
 *
 * This function creates a new string object containing the characters read
 * from the specified stream until EOF is met.
 *
 * @param stream The stream to read characters from.
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_stream(FILE *stream);

/**
 * @brief Creates a new string from a stream until a delimiter is met.
 *
 * This function creates a new string object containing the characters read
 * from the specified stream until either EOF or the specified delimiter
 * character is met.
 *
 * @param stream The stream to read characters from.
 * @param delim The delimiter character to stop reading at.
 * @return A pointer to the newly created string, or NULL if the operation failed.
 */
string *string_of_stream_delim(FILE *stream, char delim);

/**
 * @brief Resizes the string to fit the specified number of bytes.
 *
 * This function reallocates the memory used for the string to fit the
 * specified number of bytes, updating the string's stats as necessary.
 *
 * @param str A pointer to the string to be resized.
 * @param new_capacity The new capacity of the string in bytes.
 * @return A pointer to the resized string, or NULL if the operation failed.
 */
string *string_resize(string *str, size_t new_size);

/**
 * @brief Shrinks the memory used for the string to fit its contents.
 *
 * This function reallocates the memory used for the string to fit the number
 * of characters it contains.
 *
 * @param str A pointer to the string to be shrunk.
 * @return A pointer to the shrunk string, or NULL if the operation failed.
 */
string *string_shrink_alloc(string *str);

#endif
