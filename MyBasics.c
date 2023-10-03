#include <stdlib.h>
#include <string.h>
#include "MyBasics.h"

// Determines the number of characters that would be within the string equivalent of INT_MAX.
#define INT_MAX_CHARS 11

/*
 * Will discard a line of input within 'stream' until meeting a newline character or EOF.
 * Note that this will also discard the newline character.
 *
 * Use fseek(stdin, 0, SEEK_END) to clear the contents of 'stdin' instead of using this function.
 *
 * To prevent potential hanging, the loop within this function will terminate after
 * reading an FDISCARDLINE_MAX_READS number of characters from 'stream'.
 *
 * Returns 0 on success, and 1 upon failure.
 */
int fDiscardLine(FILE *stream)
{
    const static size_t MAX_READS = 50000000;
    for (size_t i = 0; i < MAX_READS; ++i)
    {
        if (getc(stream) == '\n' || feof(stream))
            return ERRCODE_SUCCESS;
    }
    return ERRCODE_GENERAL;
}

/*
 * Writes to the first argument, str, from stream until reading the delimiter
 * character, or upon the string reaching the specified length.
 *
 * \returns Returns the length of the string INCLUDING the null terminator, or 0 if nothing was written.
 *
 * \note If this function returns 0, the string will be left unmodified (that is, nothing will be written).
 *
 * \note A null terminator will replace the delimiter within the string.
 *
 * \note Otherwise, if 'length' characters were written before the delimiter was read, a null terminator will be
 * appended to the end of the string if no delimiter was read or EOF was not met.
 *
 * \note If this occurs, the returned value will be length + 1.
 *
 * \param str The pointer to string that will be written to
 * \param delim The delimiter character
 * \param length The maximum size of the string
 * \param stream The input stream to read from
 */
size_t getStr(char **str, const char delim, const size_t length, FILE *stream)
{
    // Validating the pointer to the string
    if (str == NULL)
    {
        fputs("\ngetStr(): Invalid pointer-to-pointer-to-char provided; No reading occurred\n", stderr);
        return 0;
    }

    // Validating the length argument for the string
    if (length == 0)
    {
        fprintf(stderr, "\ngetStr(): Invalid string length (%llu); No reading occurred\n", length);
        return 0;
    }

    // Validating that the input stream is, well, valid
    if (stream == NULL)
    {
        fputs("\ngetStr(): Invalid stream provided; No reading occurred\n", stderr);
        return 0;
    }

    // Ensuring stream isn't at EOF here so that we don't go through all the other stuff below
    // just to return 0 anyway
    if (feof(stream))
        return 0;

    // Reading from stream into str
    // "length + 1" to ensure there is always space for a null terminator
    char buffer[length + 1];
    size_t i = 0;
    for (; i < length; ++i)
    {
        buffer[i] = getc(stream);
        if (buffer[i] == delim || buffer[i] == EOF)
        {
            // Incrementing 'i' since we count the null terminator in the string's returned length
            buffer[i++] = '\0';
            break;
        }
    }

    // If nothing meaningful was read then we can just return 0, leaving the 'str' argument untouched.
    // This will usually occur if only the specified delimiter character was read from the 'stream'
    // which gets replaced by the null terminator.
    if (buffer[0] == '\0')
        return 0;

    // Appending a null terminator and adding it to the string's returned length, 'i', if not all of
    // the input within 'stream' was read
    if (i == length)
        buffer[i++] = '\0';

    // Allocating memory for 'str' if the user hadn't already allocated for it themselves
    if (*str == NULL)
    {
        *str = malloc(sizeof(buffer));
        // Allocation failure insurance
        if (*str == NULL)
        {
            fputs("\ngetStr(): malloc() failure; No reading occurred\n", stderr);
            return 0;
        }
    }
    strcpy_s(*str, sizeof(buffer), buffer);

    return i;
}

/*
 * Writes to the first argument, 'str', from stdin until reading a newline character,
 * or upon the string reaching the specified length.
 *
 * The difference between getStr() and this function is that this function has an
 * implicit delimiter, '\\n', an implicit stream, 'stdin', and will automatically flush
 * any unused input from stdin.
 *
 * This function has the same semantics as getStr() aside from the flushing of stdin.
 */
inline size_t getStrStdin(char **str, const size_t length)
{
    const size_t numChars = getStr(str, '\n', length, stdin);
    // If 'numChars' is greater than the passed length, that means getStr() appended a null terminator
    // due to not encountering a newline, which means there is still input that needs to be flushed,
    // hence the fseek() call below.
    if (numChars > length)
        fseek(stdin, 0, SEEK_END);
    return numChars;
}

/*
 * Returns true, if the passed char represents an alphabetical letter.
 *
 * Otherwise returns false.
 */
inline int isAlphabetical(const char letter);

/* Returns non-zero, true, if the passed char represents a number.
 *
 * Otherwise, this returns 0, false.
 */
inline int isNumerical(const char number);

/*
 * Returns non-zero, true, if the passed char represents either a number
 * or alphabetical letter.
 *
 * Returns 0, false, otherwise.
 */
inline int isAlphaNumerical(const char item);

/*
 * Returns the passed character as an integer singleton (0-9), if possible.
 *
 * If the passed character does not represent an integer, this function returns -1.
 */
inline int charToInt(const char num);

/*
 * Parses the passed string into an int which will be written to the second argument, 'num'.
 *
 * If this function fails, such as via invalid characters or overflow, 'num' will be left
 * unchanged and ERRCODE_GENERAL will be returned.
 */
int strToInt(const char *str, int *const num)
{
    if (str == NULL)
    {
        fputs("\nstrToInt(): 'str' is NULL; No conversion occurred", stderr);
        return ERRCODE_BAD_PTR;
    }
    if (num == NULL)
    {
        fputs("\nstrToInt(): 'num' is NULL; No conversion occurred", stderr);
        return ERRCODE_BAD_PTR;
    }

    const static int INT_MAX_OVERFLOW_CHK = 147483647;
    const static int INT_MIN_OVERFLOW_CHK = INT_MAX_OVERFLOW_CHK + 1;
    const static int MAX_PLACE_VALUE = 1000000000;

    const char containsSign = (*str == '-' || *str == '+');
    const char isNegative = (*str == '-');
    const char *const STR_PTR_START = str + containsSign;
    const char *const STR_PTR_END = str + strlen(str) - 1; // "strlen(str) - 1" to discount the null terminator

    int tempNum = 0; // This will replace 'num' upon the function's successful completion
    int placeValue = 1;

    // Iterating over the string in reverse allows for the correct place values to be assigned with
    // as little hassle as possible
    for (str = STR_PTR_END; str >= STR_PTR_START; --str, placeValue *= (placeValue == MAX_PLACE_VALUE) ? 1 : 10, puts("ran"))
    {
        const char currentNum = charToInt(*str);

        if (currentNum == -1)
            return ERRCODE_BAD_INPUT;

        // We can skip any values of 0
        if (currentNum == 0 && placeValue < MAX_PLACE_VALUE)
            continue;

        if (placeValue != MAX_PLACE_VALUE)
            tempNum += currentNum * placeValue;
        // Safely handling anything that might approach INT_MAX or INT_MIN
        else if (currentNum <= 2)
        {
            // Before we add the final digit, we check to see if doing so would cause an overflow
            if (tempNum - INT_MAX_OVERFLOW_CHK <= 0)
            {
                tempNum += currentNum * placeValue;
                break;
            }
            // Handling the edge case of INT_MIN being greater than INT_MAX by 1
            if (isNegative && tempNum - INT_MIN_OVERFLOW_CHK == 0)
            {
                *num = INT_MIN;
                return ERRCODE_SUCCESS;
            }
            // Catching overflow with an undefined return error code
            return ERRCODE_GENERAL;
        }
        else
            return ERRCODE_BAD_INPUT;
    }

    // If 'str' is still in its initialized value then the loop must have hit an invalid character
    // upon the first iteration, so we leave 'num' alone as per the function's guarantee and exit
    if (STR_PTR_END - str == STR_PTR_END - STR_PTR_START - 1)
        return ERRCODE_GENERAL;

    if (isNegative)
        tempNum *= -1;

    *num = tempNum;
    return ERRCODE_SUCCESS;
}

/*
 * Reads up to 11 numerical characters (including a preceding dash) and parses
 * them, writing the proper int equivalent (if any) to 'num'.
 *
 * Stops upon reading a non-numerical character
 *
 * Writes the parsed line as an int to 'num'.
 *
 * Returns 1 if the passed arguments were valid, but no characters in the stream
 * were parsable, or if type int was overflowed.
 */
int readInt(int *const num, FILE *stream)
{
    if (num == NULL)
    {
        fputs("\nreadInt(): Passed int pointer is NULL; No reading occurred", stderr);
        return ERRCODE_BAD_PTR;
    }

    if (stream == NULL)
    {
        fputs("\nreadInt(): Passed file is NULL; No reading occurred", stderr);
        return ERRCODE_BAD_FILE;
    }

    if (feof(stream))
    {
        fputs("\nreadInt(): Passed file is already at EOF; No reading occurred", stderr);
        return ERRCODE_FILE_AT_EOF;
    }

    // Buffer size of 12 (for Windows running on my Surface Go) since a typical int can hold 10 numbers
    // including a dash in the case of negative values for a total of 11 characters (INT_MAX_CHARS).
    // Then we need to account for the null terminator, so we add 1 to INT_MAX_CHARS.
    static char buffer[INT_MAX_CHARS + 1];

    int i = -1;
    // We count a single trailing dash as a valid character, so we increment 'i' just as we would for a digit
    if ((buffer[0] = getc(stream)) == '-')
        ++i;
    else
        ungetc(buffer[0], stream);

    while (i < INT_MAX_CHARS && isNumerical((buffer[++i] = getc(stream))))
        ;

    buffer[i] = '\0';

    return strToInt(buffer, num);
}
