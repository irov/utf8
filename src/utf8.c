#include "utf8/utf8.h"

#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
#define UTF8_REPLACEMENT_CHARACTER (0xFFFD)
#define UTF8_WCHAR_UTF16 (WCHAR_MAX <= 0xFFFF)
//////////////////////////////////////////////////////////////////////////
static size_t utf8_bytes_for_code_point( uint32_t cp )
{
    if( cp < 0x80U )
        return 1;
    if( cp < 0x800U )
        return 2;
    if( cp < 0x10000U )
        return 3;
    if( cp < 0x110000U )
        return 4;
    return 0;
}
//////////////////////////////////////////////////////////////////////////
static size_t utf8_encode_code_point( char * _out, size_t _capacity, size_t _offset, uint32_t cp )
{
    size_t n = utf8_bytes_for_code_point( cp );
    if( n == 0 || _offset + n > _capacity )
        return UTF8_UNKNOWN;

    if( cp < 0x80U )
    {
        _out[_offset + 0] = (char)cp;
    }
    else if( cp < 0x800U )
    {
        _out[_offset + 0] = (char)(0xC0U | (cp >> 6));
        _out[_offset + 1] = (char)(0x80U | (cp & 0x3FU));
    }
    else if( cp < 0x10000U )
    {
        _out[_offset + 0] = (char)(0xE0U | (cp >> 12));
        _out[_offset + 1] = (char)(0x80U | ((cp >> 6) & 0x3FU));
        _out[_offset + 2] = (char)(0x80U | (cp & 0x3FU));
    }
    else
    {
        _out[_offset + 0] = (char)(0xF0U | (cp >> 18));
        _out[_offset + 1] = (char)(0x80U | ((cp >> 12) & 0x3FU));
        _out[_offset + 2] = (char)(0x80U | ((cp >> 6) & 0x3FU));
        _out[_offset + 3] = (char)(0x80U | (cp & 0x3FU));
    }
    return _offset + n;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_wchar( const wchar_t * _unicode, size_t _unicodeSize, char * const _utf8, size_t _utf8Capacity )
{
    size_t unicodeSize = (_unicodeSize == UTF8_UNKNOWN) ? wcslen( _unicode ) : _unicodeSize;
    size_t utf8Size = 0;
    size_t i = 0;

    if( _unicode == NULL )
    {
        return UTF8_UNKNOWN;
    }

    if( _utf8 == NULL )
    {
        while( i != unicodeSize )
        {
            uint32_t cp;

#if UTF8_WCHAR_UTF16
            uint32_t wc = (uint32_t)_unicode[i];
            if( wc >= 0xD800U && wc <= 0xDBFFU )
            {
                if( i + 1 >= unicodeSize )
                    return UTF8_UNKNOWN;
                uint32_t low = (uint32_t)_unicode[i + 1];
                if( low < 0xDC00U || low > 0xDFFFU )
                    return UTF8_UNKNOWN;
                cp = 0x10000U + ((wc - 0xD800U) << 10) + (low - 0xDC00U);
                i += 2;
            }
            else if( wc >= 0xDC00U && wc <= 0xDFFFU )
            {
                return UTF8_UNKNOWN;
            }
            else
            {
                cp = wc;
                i += 1;
            }
#else
            cp = (uint32_t)_unicode[i];
            i += 1;
#endif

            size_t n = utf8_bytes_for_code_point( cp );
            if( n == 0 )
                return UTF8_UNKNOWN;
            utf8Size += n;
        }
        return utf8Size;
    }

    while( i != unicodeSize )
    {
        uint32_t cp;

#if UTF8_WCHAR_UTF16
        uint32_t wc = (uint32_t)_unicode[i];
        if( wc >= 0xD800U && wc <= 0xDBFFU )
        {
            if( i + 1 >= unicodeSize )
                return UTF8_UNKNOWN;
            uint32_t low = (uint32_t)_unicode[i + 1];
            if( low < 0xDC00U || low > 0xDFFFU )
                return UTF8_UNKNOWN;
            cp = 0x10000U + ((wc - 0xD800U) << 10) + (low - 0xDC00U);
            i += 2;
        }
        else if( wc >= 0xDC00U && wc <= 0xDFFFU )
        {
            return UTF8_UNKNOWN;
        }
        else
        {
            cp = wc;
            i += 1;
        }
#else
        cp = (uint32_t)_unicode[i];
        i += 1;
#endif

        size_t nextSize = utf8_encode_code_point( _utf8, _utf8Capacity, utf8Size, cp );
        if( nextSize == UTF8_UNKNOWN )
            return UTF8_UNKNOWN;
        utf8Size = nextSize;
    }

    if( utf8Size >= _utf8Capacity )
    {
        return UTF8_UNKNOWN;
    }

    _utf8[utf8Size] = '\0';

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_wchar( const char * _utf8, size_t _utf8Size, wchar_t * const _unicode, size_t _unicodeCapacity )
{
    size_t utf8Size = (_utf8Size == UTF8_UNKNOWN) ? strlen( _utf8 ) : _utf8Size;
    const char * utf8End = _utf8 + utf8Size;
    size_t unicodeSize = 0;
    const char * p;
    uint32_t cp;

    if( _utf8 == NULL )
    {
        return UTF8_UNKNOWN;
    }

    if( _unicode == NULL )
    {
        for( p = _utf8; p < utf8End; )
        {
            p = utf8_next_code( p, utf8End, &cp );
            if( p == NULL )
                return UTF8_UNKNOWN;
#if UTF8_WCHAR_UTF16
            unicodeSize += (cp > 0xFFFFU) ? 2 : 1;
#else
            unicodeSize += 1;
#endif
        }
        return unicodeSize;
    }

    for( p = _utf8; p < utf8End; )
    {
        p = utf8_next_code( p, utf8End, &cp );
        if( p == NULL )
            return UTF8_UNKNOWN;

#if UTF8_WCHAR_UTF16
        if( cp > 0xFFFFU )
        {
            if( unicodeSize + 2 > _unicodeCapacity )
                return UTF8_UNKNOWN;
            cp -= 0x10000U;
            _unicode[unicodeSize + 0] = (wchar_t)(0xD800U + (cp >> 10));
            _unicode[unicodeSize + 1] = (wchar_t)(0xDC00U + (cp & 0x3FFU));
            unicodeSize += 2;
        }
        else
        {
            if( unicodeSize + 1 > _unicodeCapacity )
                return UTF8_UNKNOWN;
            _unicode[unicodeSize] = (wchar_t)cp;
            unicodeSize += 1;
        }
#else
        if( unicodeSize + 1 > _unicodeCapacity )
            return UTF8_UNKNOWN;
        _unicode[unicodeSize] = (wchar_t)cp;
        unicodeSize += 1;
#endif
    }

    if( unicodeSize >= _unicodeCapacity )
    {
        return UTF8_UNKNOWN;
    }

    _unicode[unicodeSize] = L'\0';

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
const char * utf8_next_code( const char * _utf8, const char * _utf8End, uint32_t * _utf8Code )
{
    if( _utf8 == NULL || _utf8End == NULL || _utf8 >= _utf8End )
    {
        return NULL;
    }

    const uint8_t * p = (const uint8_t *)_utf8;
    const uint8_t * ep = (const uint8_t *)_utf8End;

    uint32_t code;
    uint32_t len;

    uint32_t b0 = p[0];

    if( (b0 & 0x80U) == 0 )
    {
        code = b0;

        len = 1;
    }
    else if( (b0 & 0xE0U) == 0xC0 )
    {
        if( b0 < 0xC2 )
        {
            return NULL;
        }

        if( p + 2 > ep )
        {
            return NULL;
        }

        uint32_t b1 = p[1];

        if( (b1 & 0xC0U) != 0x80 )
        {
            return NULL;
        }

        code = ((b0 & 0x1FU) << 6) | (b1 & 0x3FU);

        len = 2;
    }
    else if( (b0 & 0xF0U) == 0xE0 )
    {
        if( p + 3 > ep )
        {
            return NULL;
        }

        uint32_t b1 = p[1];
        uint32_t b2 = p[2];

        if( (b1 & 0xC0U) != 0x80 || (b2 & 0xC0U) != 0x80 )
        {
            return NULL;
        }        

        if( b0 == 0xE0 && (b1 & 0x3FU) < 0x20 )
        {
            return NULL;
        }

        if( b0 == 0xED && (b1 & 0x3FU) >= 0x20 )
        {
            return NULL;
        }

        code = ((b0 & 0x0FU) << 12) | ((b1 & 0x3FU) << 6) | (b2 & 0x3FU);

        len = 3;
    }
    else if( (b0 & 0xF8U) == 0xF0 )
    {
        if( b0 > 0xF4 )
        {
            return NULL;
        }

        if( p + 4 > ep )
        {
            return NULL;
        }

        uint32_t b1 = p[1];
        uint32_t b2 = p[2];
        uint32_t b3 = p[3];

        if( (b1 & 0xC0U) != 0x80 || (b2 & 0xC0U) != 0x80 || (b3 & 0xC0U) != 0x80 )
        {
            return NULL;
        }

        if( b0 == 0xF0 && (b1 & 0x3FU) < 0x10 )
        {
            return NULL;
        }

        if( b0 == 0xF4 && (b1 & 0x3FU) >= 0x10 )
        {
            return NULL;
        }

        code = ((b0 & 0x07U) << 18) | ((b1 & 0x3FU) << 12) | ((b2 & 0x3FU) << 6) | (b3 & 0x3FU);

        if( code > 0x10FFFFU )
        {
            return NULL;
        }

        len = 4;
    }
    else
    {
        return NULL;
    }

    if( code >= 0xD800U && code <= 0xDFFFU )
    {
        return NULL;
    }

    if( _utf8Code != NULL )
    {
        *_utf8Code = code;
    }

    return (const char *)(p + len);
}
//////////////////////////////////////////////////////////////////////////
const char * utf8_validate( const char * _utf8, const char * _utf8End )
{
    for( const char * p = _utf8; p < _utf8End; )
    {
        const char * next = utf8_next_code( p, _utf8End, NULL );

        if( next == NULL )
        {
            return p;
        }

        p = next;
    }

    return _utf8End;
}
//////////////////////////////////////////////////////////////////////////
static char * utf8_append_code_point( char * _out, uint32_t _cp )
{
    if( _cp < 0x80 )
    {
        *_out++ = (char)_cp;
    }
    else if( _cp < 0x800 )
    {
        *_out++ = (char)(0xC0 | (_cp >> 6));
        *_out++ = (char)(0x80 | (_cp & 0x3F));
    }
    else if( _cp < 0x10000 )
    {
        *_out++ = (char)(0xE0 | (_cp >> 12));
        *_out++ = (char)(0x80 | ((_cp >> 6) & 0x3F));
        *_out++ = (char)(0x80 | (_cp & 0x3F));
    }
    else
    {
        *_out++ = (char)(0xF0 | (_cp >> 18));
        *_out++ = (char)(0x80 | ((_cp >> 12) & 0x3F));
        *_out++ = (char)(0x80 | ((_cp >> 6) & 0x3F));
        *_out++ = (char)(0x80 | (_cp & 0x3F));
    }

    return _out;
}
//////////////////////////////////////////////////////////////////////////
const char * utf8_replace_invalid( const char * _utf8, const char * _utf8End, char * _utf8Out )
{
    for( const char * p = _utf8; p != _utf8End; )
    {
        uint32_t code;
        const char * next = utf8_next_code( p, _utf8End, &code );

        if( next != NULL )
        {
            _utf8Out = utf8_append_code_point( _utf8Out, code );

            p = next;
        }
        else
        {
            _utf8Out = utf8_append_code_point( _utf8Out, UTF8_REPLACEMENT_CHARACTER );

            p += 1;
        }
    }

    return _utf8Out;
}
//////////////////////////////////////////////////////////////////////////