#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "MyBasics.h"

#define ERRCODE_SUCCESS 0     // Code denoting a successful execution.
#define ERRCODE_DEFAULT 1     // An undefined error code; consult the function's documentation.
#define ERRCODE_NULL_PTR 2    // General code denoting a bad or NULL primitive pointer argument (eg. char* or int*).
#define ERRCODE_NULL_FILE 3   // Code denoting a bad or NULL pointer to a FILE argument.
#define ERRCODE_FILE_AT_EOF 4 // Code denoting that the given FILE is at EOF *before* any reading occurred.

// Used by strToInt() and readInt().
// Determines the number of characters that would be within the string equivalent of INT_MAX.
#define INT_MAX_CHARS 11

// Used by strToInt()
#define INT_MAX_PLACEVALUE 1000000000

/*
 * Will discard a line of input within *stream up to a newline character.
 * Will discard the newline character as well.
 *
 * Use fseek(stdin, 0, SEEK_END) to clear the contents of 'stdin' instead of using this function.
 *
 * To prevent potential hanging, the loop within this function will terminate after
 * 2000000 character reads.
 *
 * Returns 0 on success, and 1 upon failure.
 */
inline int fDiscardLine(FILE *stream)
{
    static const int MAX_NUM_CHARS = 2000000;

    for (int i = 0; i < MAX_NUM_CHARS; i++)
    {
        if (getc(stream) == '\n' || feof(stream))
            return ERRCODE_SUCCESS;
    }
    return ERRCODE_DEFAULT;
}

/*
 * Writes to the first argument, str, from stream until reading the delimiter character, or upon the
 * string reaching the specified length.
 *
 * A null terminator will replace the delimiter within the string.
 *
 * Otherwise, if applicable and possible, a null terminator will be appended to the end of the string
 * if no delimiter was read or EOF was not met. If this occurs, the returned value will be length + 1.
 *
 * Returns the length of the string INCLUDING the null terminator, or -1 upon an error.
 * Returns 0 if stream is at EOF, or if the first character read is the specified delimiter.
 */
size_t getStr(char **str, const char delim, const size_t length, FILE *stream)
{
    // Validating the pointer to the string
    if (str == NULL)
    {
        fprintf(stderr, "\ngetStr(): Invalid pointer-to-pointer-to-char provided; No reading occurred\n");
        return -1;
    }

    // Validating the length argument for the string
    if (length <= 0)
    {
        fprintf(stderr, "\ngetStr(): Invalid string length: %lld; No reading occurred\n", length);
        return -1;
    }

    // Validating that the input stream is, well, valid
    if (stream == NULL)
    {
        fprintf(stderr, "\ngetStr(): Invalid stream provided; No reading occurred\n");
        return -1;
    }

    // Ensuring stream isn't at EOF here so that we don't go through all the other stuff below.
    // We just return 0 since there aren't any actual problems with the arguments; it's just that
    // the stream provided is at EOF, and having a separate return code for this could be useful.
    if (feof(stream))
        return 0;

    // Reading from stream into str
    // "length + 1" to ensure there is always space for a null terminator
    char buffer[length + 1];
    unsigned i = 0;
    for (; i < length; i++)
    {
        buffer[i] = getc(stream);
        if (buffer[i] == delim || buffer[i] == EOF)
        {
            // Incrementing 'i' since we count the null terminator in the string's returned length
            buffer[i++] = '\0';
            break;
        }
    }

    // If nothing was read, we can just return 0, leaving the 'str' argument untouched
    // This will usually occur if only the specified delimiter character was read from the 'stream'
    // which gets replaced by the null terminator
    if (buffer[0] == '\0')
    {
        return 0;
    }

    // Appending a null terminator and adding it to the string's returned length, 'i', if not all of
    // the input within 'stream' was read
    if (i == length)
    {
        buffer[i++] = '\0';
    }

    // Allocating memory for 'str' if the user hadn't already allocated for it themselves
    if (*str == NULL)
    {
        *str = malloc(sizeof(char) * i);
        // Allocation failure insurance
        if (*str == NULL)
        {
            fprintf(stderr, "\ngetStr(): malloc() failure; No reading occurred\n");
            return -1;
        }
    }
    strcpy_s(*str, sizeof(char) * i, buffer);

    return i;
}

/*
 * Writes to the first argument, str, from stdin until reading a newline character, or upon the
 * string reaching the specified length.
 *
 * The difference between getStr() and this function is that this function has an implicit delimiter,
 * '\\n', an implicit stream, 'stdin', and will automatically flush any unused input from stdin.
 *
 * A null terminator will replace the newline within the string.
 *
 * Otherwise, if the input exceeds the passed length, a null terminator will be APPENDED to the
 * end of the string if no newline was read or EOF was not met. If this occurs, the returned value
 * will be length + 1.
 *
 * Returns the length of the string INCLUDING the null terminator, or -1 upon an error.
 * Returns 0 if the first character read is a newline character.
 */
inline int getStrStdin(char **str, const size_t length)
{
    const size_t numChars = getStr(str, '\n', length, stdin);
    // If 'numChars' is greater than the passed length, that means getStr() appended a null terminator
    // due to not encountering a newline, which means there is still input that needs to be flushed,
    // hence the fseek() call below.
    if (numChars > length)
        fseek(stdin, 0, SEEK_END);
    return numChars;
}

/* Returns the first index of the passed character within the passed string after
 * offset.
 *
 * Returns -1 if the character is not present within the string.
 */
int indexOf(const char *str, const char letter, const size_t offset)
{
    const size_t LENGTH_OF_STR = strlen(str);
    // Ensuring offset is valid
    if (offset > LENGTH_OF_STR || offset < 0)
    {
        fprintf(stderr, "\nindexOf(): Invalid offset for passed string: %llu; No reading occurred\n", offset);
        return -1;
    }

    // Searching through the string for the character
    //
    char currentChar;
    for (size_t i = offset; i < LENGTH_OF_STR && (currentChar = str[i]) != '\0'; i++)
    {
        if (currentChar == letter)
        {
            return i;
        }
    }
    return -1;
}

/*
 * Returns non-zero, true, if the passed char represents an alphabetical letter.
 *
 * Otherwise, returns 0, false.
 */
inline int isAlphabetical(const char letter)
{
    return ((letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z'));
}

/* Returns non-zero, true, if the passed char represents a number.
 *
 * Otherwise, this returns 0, false.
 */
inline int isNumerical(const char number)
{
    return (number >= '0' && number <= '9');
}

/*
 * Returns non-zero, true, if the passed char represents either a number or
 * alphabetical letter.
 *
 * Returns 0, false, otherwise.
 */
inline int isAlphaNumerical(const char item)
{
    return (isAlphabetical(item) || isNumerical(item));
}

/*
 * Returns the passed character as an integer singleton (0-9), if possible.
 *
 * If the passed character does not represent an integer, this function returns -1.
 */
inline short int charToInt(const char num)
{
    if (isNumerical(num))
        return num - '0';
    return -1;
}

/*
 * Parses the passed string into an int which will be written to the second argument, 'num'.
 *
 * If this function fails, 'num' will be left unchanged and an error code returned.
 *
 * Returns 1 if the passed arguments were valid, but no characters in the stream were valid, or if
 * type 'int' was overflowed.
 *
 * Otherwise, this function returns 0 on success.
 */
int strToInt(const char *str, int *num)
{
    // Validating the string's existence
    if (str == NULL)
    {
        fprintf(stderr, "\nstrToInt(): Invalid string passed; No conversion occurred\n");
        return ERRCODE_NULL_PTR;
    }

    // We can use shorts because the length of str should be no longer than INT_MAX_CHARS
    const unsigned short STR_SIZE = strlen(str);

    // The expression below checks if there is a leading dash sign within the string which tells the
    // later parsing loop to ignore this character
    // Also used as a boolean to convert the resultant int to a negative if applicable
    const unsigned short isNegative = (*str == '-');
    if (STR_SIZE == 0)
    {
        fprintf(stderr, "\nstrToInt(): Invalid string length: %u; No conversion occurred\n", STR_SIZE);
        return ERRCODE_DEFAULT;
    }
    else if (STR_SIZE == 1 && isNegative)
    {
        return ERRCODE_DEFAULT;
    }

    // Removing any leading zeros
    unsigned short endIndex = isNegative;
    while (endIndex < STR_SIZE && str[endIndex] == '0')
    {
        endIndex++;
    }

    // If, for whatever reason, "-0" is the passed string, we handle that below since the above loop
    // would normally discard it
    if (endIndex == STR_SIZE - 1 && str[endIndex] == '0')
    {
        *num = 0;
        return ERRCODE_SUCCESS;
    }

    // Checking to ensure that the string isn't long enough to definitively overflow int
    if (STR_SIZE - endIndex > INT_MAX_CHARS)
    {
        fprintf(stderr, "\nstrToInt(): Invalid string length: %u; No conversion occurred\n", STR_SIZE);
        return ERRCODE_DEFAULT;
    }

    // This, assuming no issues arise while parsing the passed string, will replace the value at 'num'
    int tempNum = 0;
    int placeValue = 1;

    // Iterating over 'str' in reverse so we can assign the correct place values with as little hassle
    // as possible
    // "STR_SIZE - 1" to skip the null terminator
    short i = STR_SIZE - 1;
    for (; i >= endIndex; i--)
    {
        // We can skip over any values of 0
        if (str[i] == '0')
        {
            placeValue *= 10;
            continue;
        }

        if (isNumerical(str[i]))
        {
            // Ensuring no overflow will occur
            if (tempNum - 147483647 <= 0)
            {
                tempNum += charToInt(str[i]) * placeValue;
                placeValue *= 10;
            }
        }
        else // Breaking upon hitting anything non-numeric
        {
            break;
        }
    }

    // If 'i' is still its initialized value, that means the loop had hit an invalid character, so
    // we leave 'num' alone and return an error code
    if (i == STR_SIZE - 1)
        return ERRCODE_DEFAULT;

    // Finally, we handle the negative sign if necessary
    if (isNegative)
        tempNum *= -1;

    *num = tempNum;
    return ERRCODE_SUCCESS;
}

/*
 * Reads up to 11 numerical characters (including a preceding dash) and parses them, writing the proper int equivalent
 * (if any) to 'num'.
 * Stops upon reading a non-numerical character
 *
 * Writes the parsed line as an int to 'num'.
 *
 * Returns 1 if the passed arguments were valid, but no characters in the stream were parsable,
 * or if type int was overflowed.
 */
int readInt(int *num, FILE *stream)
{
    // 'num' pointer validation
    if (num == NULL)
    {
        fprintf(stderr, "\nreadInt(): Passed int pointer is NULL; No reading occurred\n");
        return ERRCODE_NULL_PTR;
    }

    // 'stream' validation
    if (stream == NULL)
    {
        fprintf(stderr, "\nreadInt(): Passed file is NULL; No reading occurred\n");
        return ERRCODE_NULL_FILE;
    }

    // Preliminary check to ensure 'stream' isn't already at EOF
    if (feof(stream))
    {
        fprintf(stderr, "\nreadInt(): Passed file is already at EOF; No reading occurred\n");
        return ERRCODE_FILE_AT_EOF;
    }

    // Buffer size of 12 (for Windows running on my Surface Go) since a typical int can hold 10 numbers
    // including a dash in the case of negative values for a total of 11 characters (INT_MAX_CHARS).
    // Then we need to account for the null terminator, so we add 1 to INT_MAX_CHARS.
    static const unsigned short BUFFER_SIZE = sizeof(char) * (INT_MAX_CHARS + 1);
    char buffer[BUFFER_SIZE];

    int i = -1;
    // Handling the trailing dash if present
    if ((buffer[0] = getc(stdin)) == '-')
    {
        ++i; // We count the dash as a valid character, so we increment 'i' just as we would for a digit
    }
    else // Restore the character otherwise since it could be a valid digit
    {
        ungetc(buffer[0], stdin);
    }

    while (i < INT_MAX_CHARS && isNumerical((buffer[++i] = getc(stream))))
        ;

    // Setting the null terminator below
    buffer[i] = '\0';

    return strToInt(buffer, num);
}
