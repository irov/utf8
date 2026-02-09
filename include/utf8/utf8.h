#ifndef UTF8_H_
#define UTF8_H_

#include <wchar.h>
#include <uchar.h>
#include <stddef.h>
#include <stdint.h>

#define UTF8_UNKNOWN ((size_t)-1)

/**
 * Returns the number of UTF-8 bytes required to encode the wide-character string.
 *
 * @param _unicode     Input wide-character string.
 * @param _unicodeSize Number of wchar_t elements, or UTF8_UNKNOWN for wcslen().
 * @return Required UTF-8 byte count (excluding NUL), or UTF8_UNKNOWN on invalid input.
 */
size_t utf8_from_wchar_size( const wchar_t * _unicode, size_t _unicodeSize );

/**
 * Converts a wide-character string to UTF-8.
 *
 * @param _unicode      Input wide-character string.
 * @param _unicodeSize  Number of wchar_t elements, or UTF8_UNKNOWN for wcslen().
 * @param _utf8         Output buffer.
 * @param _utf8Capacity Output buffer size in bytes (including trailing NUL).
 * @return Number of UTF-8 bytes written (excluding NUL), or UTF8_UNKNOWN on error.
 */
size_t utf8_from_wchar( const wchar_t * _unicode, size_t _unicodeSize, char * const _utf8, size_t _utf8Capacity );

/**
 * Returns the number of wchar_t required to decode the UTF-8 string.
 *
 * @param _utf8     Input UTF-8 string.
 * @param _utf8Size Number of bytes, or UTF8_UNKNOWN for strlen().
 * @return Required wchar_t count (excluding L'\0'), or UTF8_UNKNOWN on invalid UTF-8.
 */
size_t utf8_to_wchar_size( const char * _utf8, size_t _utf8Size );

/**
 * Converts a UTF-8 string to wide characters.
 *
 * @param _utf8            Input UTF-8 string.
 * @param _utf8Size        Number of bytes, or UTF8_UNKNOWN for strlen().
 * @param _unicode         Output buffer.
 * @param _unicodeCapacity Output buffer size in wchar_t elements (including L'\0').
 * @return Number of wchar_t written (excluding L'\0'), or UTF8_UNKNOWN on error.
 */
size_t utf8_to_wchar( const char * _utf8, size_t _utf8Size, wchar_t * const _unicode, size_t _unicodeCapacity );

/**
 * Encodes a single Unicode code point (U+0000..U+10FFFF) to UTF-8.
 *
 * @param _code Code point (UTF-32).
 * @param _utf8 Output buffer; must have space for at least 5 bytes (4 UTF-8 + NUL).
 *
 * @return Number of UTF-8 bytes written (1..4), or UTF8_UNKNOWN if _code > 0x10FFFF.
 */
size_t utf8_from_char32( char32_t _code, char _utf8[5] );

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