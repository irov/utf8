#ifndef UTF8_H_
#define UTF8_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UTF8_UNKNOWN ((size_t)-1)

/**
 * Converts a wide-character string to UTF-8.
 *
 * @param _unicode      Input wide-character string.
 * @param _unicodeSize  Number of wchar_t elements, or UTF8_UNKNOWN for wcslen().
 * @param _utf8         Output buffer; pass NULL for size-only mode.
 * @param _utf8Capacity Output buffer size in bytes (including trailing NUL).
 *
 * @return Number of UTF-8 bytes written (excluding NUL), or required size if
 *         _utf8 is NULL. UTF8_UNKNOWN on error (invalid input or buffer overflow).
 */
size_t utf8_from_wchar( const wchar_t * _unicode, size_t _unicodeSize, char * const _utf8, size_t _utf8Capacity );

/**
 * Converts a UTF-8 string to wide characters.
 *
 * @param _utf8           Input UTF-8 string.
 * @param _utf8Size       Number of bytes, or UTF8_UNKNOWN for strlen().
 * @param _unicode        Output buffer; pass NULL for size-only mode.
 * @param _unicodeCapacity Output buffer size in wchar_t elements (including trailing L'\0').
 *
 * @return Number of wchar_t written (excluding L'\0'), or required count if
 *         _unicode is NULL. UTF8_UNKNOWN on error (invalid UTF-8 or buffer overflow).
 */
size_t utf8_to_wchar( const char * _utf8, size_t _utf8Size, wchar_t * const _unicode, size_t _unicodeCapacity );

/**
 * Reads and validates the next UTF-8 code point.
 *
 * @param _utf8     Current position in UTF-8 string.
 * @param _utf8End  End of sequence (one-past-last byte).
 * @param _utf8Code Optional: where to store decoded code point (U+0000..U+10FFFF).
 *
 * @return Pointer to the byte after the decoded code point, or NULL on error
 *         (invalid UTF-8, empty range, or NULL arguments).
 */
const char * utf8_next_code( const char * _utf8, const char * _utf8End, uint32_t * const _utf8Code );

/**
 * Checks whether the byte range [_utf8, _utf8End) is valid UTF-8.
 *
 * @param _utf8    Start of UTF-8 sequence.
 * @param _utf8End End of sequence (one-past-last byte).
 *
 * @return _utf8End if the entire range is valid; otherwise pointer to the
 *         first invalid byte.
 */
const char * utf8_validate( const char * _utf8, const char * _utf8End );

/**
 * Replaces invalid UTF-8 sequences with U+FFFD (replacement character).
 *
 * @param _utf8    Start of input UTF-8 sequence.
 * @param _utf8End End of input sequence (one-past-last byte).
 * @param _utf8Out Output buffer (worst case ~3x input size).
 *
 * @return Pointer to the byte after the last byte written, or NULL on error.
 */
const char * utf8_replace_invalid( const char * _utf8, const char * _utf8End, char * _utf8Out );

#endif