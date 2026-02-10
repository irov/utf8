#include "utf8/utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TEST(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            return 1; \
        } \
    } while (0)

static int test_utf8_next_code( void )
{
    uint32_t cp;
    const char * p;
    const char * end;

    /* ASCII */
    p = "A";
    end = p + 1;
    TEST( utf8_next_code( p, p, &cp ) == NULL );
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && p == end && cp == 'A' );

    /* 2-byte UTF-8: Cyrillic 'p' U+043F */
    p = "\xD0\xBF";
    end = p + 2;
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && p == end && cp == 0x043F );

    /* 3-byte UTF-8: Japanese 'day' U+65E5 */
    p = "\xE6\x97\xA5";
    end = p + 3;
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && p == end && cp == 0x65E5 );

    /* 4-byte UTF-8: U+10346 */
    p = "\xF0\x90\x8D\x86";
    end = p + 4;
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && p == end && cp == 0x10346 );

    /* Invalid: lone continuation 0x80 */
    p = "\x80";
    end = p + 1;
    TEST( utf8_next_code( p, end, &cp ) == NULL );

    /* Invalid: overlong 2-byte for U+0001 (must use 1 byte) */
    p = "\xC0\x81";
    end = p + 2;
    TEST( utf8_next_code( p, end, &cp ) == NULL );

    /* Invalid: surrogate U+D800 encoded as UTF-8 (ED A0 80) */
    p = "\xED\xA0\x80";
    end = p + 3;
    TEST( utf8_next_code( p, end, &cp ) == NULL );

    /* Invalid: truncated sequence */
    p = "\xE6\x97";
    end = p + 2;
    TEST( utf8_next_code( p, end, &cp ) == NULL );

    /* Multiple code points */
    p = "Hi";
    end = p + 2;
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && cp == 'H' );
    p = utf8_next_code( p, end, &cp );
    TEST( p != NULL && cp == 'i' );
    TEST( utf8_next_code( p, end, &cp ) == NULL );

    return 0;
}

static int test_utf8_validate( void )
{
    const char * s;
    const char * end;

    /* Valid ASCII */
    s = "hello";
    end = s + 5;
    TEST( utf8_validate( s, end ) == end );

    /* Valid UTF-8 */
    s = "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82"; /* "привет" */
    end = s + 12;
    TEST( utf8_validate( s, end ) == end );

    /* Invalid: returns pointer to first invalid byte */
    s = "hel\x80lo";
    end = s + 6;
    TEST( utf8_validate( s, end ) == s + 3 );

    /* Empty range */
    s = "x";
    TEST( utf8_validate( s, s ) == s );

    return 0;
}

static int test_utf8_replace_invalid( void )
{
    char buf[64];
    const char * in;
    const char * inEnd;
    char * out;

    /* Valid string: no change */
    in = "hello";
    inEnd = in + 5;
    memset( buf, 0, sizeof( buf ) );
    out = (char *)utf8_replace_invalid( in, inEnd, buf );
    TEST( out != NULL && out == buf + 5 );
    TEST( memcmp( buf, "hello", 5 ) == 0 );

    /* Invalid byte replaced with U+FFFD (3 bytes) */
    in = "a" "\x80" "b";
    inEnd = in + 3;
    memset( buf, 0, sizeof( buf ) );
    out = (char *)utf8_replace_invalid( in, inEnd, buf );
    TEST( out != NULL );
    TEST( buf[0] == 'a' );
    TEST( (unsigned char)buf[1] == 0xEF && (unsigned char)buf[2] == 0xBF && (unsigned char)buf[3] == 0xBD );
    TEST( buf[4] == 'b' );

    return 0;
}

static int test_utf8_from_unicodez( void )
{
    wchar_t wbuf[32];
    char buf[64];
    size_t n;

    /* ASCII */
    wbuf[0] = L'h';
    wbuf[1] = L'i';
    wbuf[2] = L'\0';
    /* ptr+size API */
    n = utf8_from_unicodez( wbuf, 2, buf, sizeof( buf ) );
    TEST( n == 2 );
    TEST( buf[0] == 'h' && buf[1] == 'i' );

    /* Size calculation (ptr+size) */
    n = utf8_from_unicodez_size( wbuf, 2 );
    TEST( n == 2 );

    /* Cyrillic "при" - 3 chars, 6 UTF-8 bytes */
    wbuf[0] = 0x043F; /* п */
    wbuf[1] = 0x0440; /* р */
    wbuf[2] = 0x0438; /* и */
    wbuf[3] = L'\0';
    n = utf8_from_unicodez( wbuf, 3, buf, sizeof( buf ) );
    TEST( n == 6 );
    TEST( (unsigned char)buf[0] == 0xD0 && (unsigned char)buf[1] == 0xBF );
    TEST( (unsigned char)buf[2] == 0xD1 && (unsigned char)buf[3] == 0x80 );
    TEST( (unsigned char)buf[4] == 0xD0 && (unsigned char)buf[5] == 0xB8 );

    /* Insufficient capacity */
    n = utf8_from_unicodez( wbuf, 3, buf, 2 );
    TEST( n == UTF8_UNKNOWN );

    /* Invalid: surrogate in input (not a valid Unicode scalar value) */
    wbuf[0] = (wchar_t)0xD800;
    wbuf[1] = L'\0';
    n = utf8_from_unicodez( wbuf, 1, buf, sizeof( buf ) );
    TEST( n == UTF8_UNKNOWN );

    return 0;
}

static int test_utf8_to_unicodez( void )
{
    char buf[64];
    wchar_t wbuf[32];
    size_t n;

    /* ASCII */
    memcpy( buf, "Hi", 3 );
    /* ptr+size API */
    n = utf8_to_unicodez( buf, 2, wbuf, sizeof( wbuf ) / sizeof( wchar_t ) );
    TEST( n == 2 );
    TEST( wbuf[0] == L'H' && wbuf[1] == L'i' );

    /* Size calculation (ptr+size) */
    n = utf8_to_unicodez_size( buf, 2 );
    TEST( n == 2 );

    /* Cyrillic "при" */
    memcpy( buf, "\xD0\xBF\xD1\x80\xD0\xB8", 6 );
    buf[6] = '\0';
    n = utf8_to_unicodez( buf, 6, wbuf, sizeof( wbuf ) / sizeof( wchar_t ) );
    TEST( n == 3 );
    TEST( wbuf[0] == 0x043F && wbuf[1] == 0x0440 && wbuf[2] == 0x0438 );

    /* Invalid UTF-8 */
    buf[0] = '\x80';
    buf[1] = '\0';
    n = utf8_to_unicodez( buf, 1, wbuf, sizeof( wbuf ) / sizeof( wchar_t ) );
    TEST( n == UTF8_UNKNOWN );

    return 0;
}

static int test_utf8_from_unicode32( void )
{
    char buf[5];
    size_t n;

    /* 1-byte: ASCII */
    n = utf8_from_unicode32_symbol( 0x00, buf );
    TEST( n == 1 && buf[0] == '\0' );
    n = utf8_from_unicode32_symbol( 'A', buf );
    TEST( n == 1 && buf[0] == 'A' );
    n = utf8_from_unicode32_symbol( 0x7F, buf );
    TEST( n == 1 && (unsigned char)buf[0] == 0x7F );

    /* 2-byte: U+043F Cyrillic 'п' */
    n = utf8_from_unicode32_symbol( 0x043F, buf );
    TEST( n == 2 && (unsigned char)buf[0] == 0xD0 && (unsigned char)buf[1] == 0xBF );

    /* 3-byte: U+65E5 Japanese '日' */
    n = utf8_from_unicode32_symbol( 0x65E5, buf );
    TEST( n == 3 && (unsigned char)buf[0] == 0xE6 && (unsigned char)buf[1] == 0x97 && (unsigned char)buf[2] == 0xA5 );

    /* 4-byte: U+10346, U+10FFFF */
    n = utf8_from_unicode32_symbol( 0x10346, buf );
    TEST( n == 4 && (unsigned char)buf[0] == 0xF0 && (unsigned char)buf[1] == 0x90 && (unsigned char)buf[2] == 0x8D && (unsigned char)buf[3] == 0x86 );
    n = utf8_from_unicode32_symbol( 0x10FFFF, buf );
    TEST( n == 4 && (unsigned char)buf[0] == 0xF4 && (unsigned char)buf[3] == 0xBF );

    /* Invalid: surrogates U+D800..U+DFFF */
    n = utf8_from_unicode32_symbol( 0xD800, buf );
    TEST( n == UTF8_UNKNOWN );
    n = utf8_from_unicode32_symbol( 0xDFFF, buf );
    TEST( n == UTF8_UNKNOWN );

    /* Invalid: beyond Unicode */
    n = utf8_from_unicode32_symbol( 0x110000, buf );
    TEST( n == UTF8_UNKNOWN );
    n = utf8_from_unicode32_symbol( 0xFFFFFFFF, buf );
    TEST( n == UTF8_UNKNOWN );

    return 0;
}

static int test_roundtrip( void )
{
    const char * utf8 = "Hello, \xD0\x9C\xD0\xB8\xD1\x80!"; /* "Hello, Мир!" */
    size_t utf8Len = strlen( utf8 );
    wchar_t wbuf[64];
    char buf[128];
    size_t n;

    n = utf8_to_unicodez( utf8, utf8Len, wbuf, 64 );
    TEST( n != UTF8_UNKNOWN );
    wbuf[n] = L'\0';

    n = utf8_from_unicodez( wbuf, n, buf, sizeof( buf ) );
    TEST( n != UTF8_UNKNOWN );
    buf[n] = '\0';

    TEST( n == utf8Len && memcmp( buf, utf8, utf8Len ) == 0 );

    return 0;
}

int main( void )
{
    int failed = 0;

    failed += test_utf8_next_code();
    failed += test_utf8_validate();
    failed += test_utf8_replace_invalid();
    failed += test_utf8_from_unicodez();
    failed += test_utf8_to_unicodez();
    failed += test_utf8_from_unicode32();
    failed += test_roundtrip();

    if( failed == 0 )
    {
        printf( "All tests passed.\n" );
        return 0;
    }
    fprintf( stderr, "%d test(s) failed.\n", failed );
    return 1;
}
