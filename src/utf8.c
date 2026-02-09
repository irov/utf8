#include "utf8/utf8.h"

#include <stdint.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
#define UTF8_REPLACEMENT_CHARACTER (0xFFFD)
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_wchar( const wchar_t * _unicode, size_t _unicodeSize, char * const _utf8, size_t _utf8Capacity )
{
    if( _utf8 != NULL && _utf8Capacity == 0 )
    {
        return 0;
    }

    size_t unicodeSize = (_unicodeSize == UTF8_UNKNOWN) ? wcslen( _unicode ) : _unicodeSize;

    size_t utf8Size = 0;

    if( _utf8 == NULL )
    {
        for( size_t i = 0; i != unicodeSize; ++i )
        {
            uint32_t wc = (uint32_t)_unicode[i];

            if( wc < 0x80 )
            {
                utf8Size += 1;
            }
            else if( wc < 0x800 )
            {
                utf8Size += 2;
            }
            else if( wc < 0x10000 )
            {
                utf8Size += 3;
            }
            else if( wc < 0x110000 )
            {
                utf8Size += 4;
            }
            else
            {
                return UTF8_UNKNOWN;
            }
        }

        return utf8Size;
    }

    for( size_t i = 0; i != unicodeSize; ++i )
    {
        uint32_t wc = (uint32_t)_unicode[i];

        if( wc < 0x80 )
        {
            if( utf8Size + 0 >= _utf8Capacity )
            {
                break;
            }

            _utf8[utf8Size + 0] = (char)wc;

            utf8Size += 1;
        }
        else if( wc < 0x800 )
        {
            if( utf8Size + 1 >= _utf8Capacity )
            {
                break;
            }

            _utf8[utf8Size + 0] = (char)(0xC0 | (wc >> 6));
            _utf8[utf8Size + 1] = (char)(0x80 | (wc & 0x3F));

            utf8Size += 2;
        }
        else if( wc < 0x10000 )
        {
            if( utf8Size + 2 >= _utf8Capacity )
            {
                break;
            }

            _utf8[utf8Size + 0] = (char)(0xE0 | (wc >> 12));
            _utf8[utf8Size + 1] = (char)(0x80 | ((wc >> 6) & 0x3F));
            _utf8[utf8Size + 2] = (char)(0x80 | (wc & 0x3F));

            utf8Size += 3;
        }
        else if( wc < 0x110000 )
        {
            if( utf8Size + 3 >= _utf8Capacity )
            {
                break;
            }

            _utf8[utf8Size + 0] = (char)(0xF0 | (wc >> 18));
            _utf8[utf8Size + 1] = (char)(0x80 | ((wc >> 12) & 0x3F));
            _utf8[utf8Size + 2] = (char)(0x80 | ((wc >> 6) & 0x3F));
            _utf8[utf8Size + 3] = (char)(0x80 | (wc & 0x3F));

            utf8Size += 4;
        }
        else
        {
            return UTF8_UNKNOWN;
        }
    }

    if( utf8Size + 1 > _utf8Capacity )
    {
        return UTF8_UNKNOWN;
    }

    _utf8[utf8Size] = '\0';

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_wchar( const char * _utf8, size_t _utf8Size, wchar_t * const _unicode, size_t _unicodeCapacity )
{
    if( _unicode != NULL && _unicodeCapacity == 0 )
    {
        return 0;
    }

    size_t utf8Size = (_utf8Size == UTF8_UNKNOWN) ? strlen( _utf8 ) : _utf8Size;

    size_t unicodeSize = 0;

    if( _unicode == NULL )
    {
        for( size_t i = 0; i != utf8Size; )
        {
            uint32_t c = (uint8_t)_utf8[i];

            if( (c & 0x80) == 0 )
            {
                i += 1;
            }
            else if( (c & 0xE0) == 0xC0 )
            {
                if( i + 1 >= utf8Size )
                {
                    return UTF8_UNKNOWN;
                }

                i += 2;
            }
            else if( (c & 0xF0) == 0xE0 )
            {
                if( i + 2 >= utf8Size )
                {
                    return UTF8_UNKNOWN;
                }

                i += 3;
            }
            else if( (c & 0xF8) == 0xF0 )
            {
                if( i + 3 >= utf8Size )
                {
                    return UTF8_UNKNOWN;
                }

                i += 4;
            }
            else
            {
                return UTF8_UNKNOWN;
            }

            unicodeSize += 1;
        }

        return unicodeSize;
    }

    for( size_t i = 0; i != utf8Size; )
    {
        if( unicodeSize >= _unicodeCapacity )
        {
            break;
        }

        uint8_t c0 = (uint8_t)_utf8[i + 0];

        if( (c0 & 0x80) == 0 )
        {
            _unicode[unicodeSize] = (wchar_t)c0;

            i += 1;
        }
        else if( (c0 & 0xE0) == 0xC0 )
        {
            if( i + 1 >= utf8Size )
            {
                return UTF8_UNKNOWN;
            }

            uint8_t c1 = (uint8_t)_utf8[i + 1];

            _unicode[unicodeSize] = (wchar_t)((c0 & 0x1F) << 6) | (c1 & 0x3F);

            i += 2;
        }
        else if( (c0 & 0xF0) == 0xE0 )
        {
            if( i + 2 >= utf8Size )
            {
                return UTF8_UNKNOWN;
            }

            uint8_t c1 = (uint8_t)_utf8[i + 1];
            uint8_t c2 = (uint8_t)_utf8[i + 2];

            _unicode[unicodeSize] = (wchar_t)((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);

            i += 3;
        }
        else if( (c0 & 0xF8) == 0xF0 )
        {
            if( i + 3 >= utf8Size )
            {
                return UTF8_UNKNOWN;
            }

            uint8_t c1 = (uint8_t)_utf8[i + 1];
            uint8_t c2 = (uint8_t)_utf8[i + 2];
            uint8_t c3 = (uint8_t)_utf8[i + 3];

            _unicode[unicodeSize] = (wchar_t)((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);

            i += 4;
        }
        else
        {
            return UTF8_UNKNOWN;
        }

        unicodeSize += 1;
    }

    _unicode[unicodeSize] = L'\0';

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_char32( char32_t _code, char _utf8[5] )
{
    if( _code <= 0x7FU )
    {
        _utf8[0] = (char)_code;
        _utf8[1] = '\0';

        return 1;
    }
    else if( _code <= 0x7FFU )
    {
        _utf8[0] = (char)(0xC0U | ((_code >> 6) & 0x1FU));
        _utf8[1] = (char)(0x80U | (_code & 0x3FU));
        _utf8[2] = '\0';

        return 2;
    }
    else if( _code <= 0xFFFFU )
    {
        _utf8[0] = (char)(0xE0U | ((_code >> 12) & 0x0FU));
        _utf8[1] = (char)(0x80U | ((_code >> 6) & 0x3FU));
        _utf8[2] = (char)(0x80U | (_code & 0x3FU));
        _utf8[3] = '\0';

        return 3;
    }
    else if( _code <= 0x10FFFFU )
    {
        _utf8[0] = (char)(0xF0U | ((_code >> 18) & 0x07U));
        _utf8[1] = (char)(0x80U | ((_code >> 12) & 0x3FU));
        _utf8[2] = (char)(0x80U | ((_code >> 6) & 0x3FU));
        _utf8[3] = (char)(0x80U | (_code & 0x3FU));
        _utf8[4] = '\0';

        return 4;
    }
    
    return UTF8_UNKNOWN;
}
//////////////////////////////////////////////////////////////////////////
const char * utf8_next_code( const char * _utf8, const char * _utf8End, uint32_t * _utf8Code )
{
    if( _utf8 == _utf8End )
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
        if( p + 2 > ep )
        {
            return NULL;
        }

        uint32_t p1 = p[1];

        code = ((b0 & 0x1FU) << 6) | (p1 & 0x3FU);

        len = 2;
    }
    else if( (b0 & 0xF0U) == 0xE0 )
    {
        if( p + 3 > ep )
        {
            return NULL;
        }

        uint32_t p1 = p[1];
        uint32_t p2 = p[2];

        code = ((b0 & 0x0FU) << 12) | ((p1 & 0x3FU) << 6) | (p2 & 0x3FU);

        len = 3;
    }
    else if( (b0 & 0xF8U) == 0xF0 )
    {
        if( p + 4 > ep )
        {
            return NULL;
        }

        uint32_t p1 = p[1];
        uint32_t p2 = p[2];
        uint32_t p3 = p[3];

        code = ((b0 & 0x07U) << 18) | ((p[1] & 0x3FU) << 12) | ((p[2] & 0x3FU) << 6) | (p[3] & 0x3FU);

        len = 4;
    }
    else
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
static char * __append_code_point( char * _out, uint32_t _cp )
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
            _utf8Out = __append_code_point( _utf8Out, code );

            p = next;
        }
        else
        {
            _utf8Out = __append_code_point( _utf8Out, UTF8_REPLACEMENT_CHARACTER );

            p += 1;
        }
    }

    return _utf8Out;
}
//////////////////////////////////////////////////////////////////////////