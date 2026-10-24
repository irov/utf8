#include "utf8/utf8.h"

//////////////////////////////////////////////////////////////////////////
#define UTF8_REPLACEMENT_CHARACTER (0xFFFD)
#define UTF8_SURROGATE_LO          (0xD800)
#define UTF8_SURROGATE_LEAD_HI     (0xDBFF)
#define UTF8_SURROGATE_TRAIL_LO    (0xDC00)
#define UTF8_SURROGATE_HI          (0xDFFF)
#define UTF8_MAX_CODE_POINT        (0x10FFFF)
#define UTF8_WCHAR_SIZE            (sizeof( wchar_t ))
//////////////////////////////////////////////////////////////////////////
static size_t __utf8_code_size( uint32_t _code )
{
    if( _code < 0x80 )
    {
        return 1;
    }
    else if( _code < 0x800 )
    {
        return 2;
    }
    else if( _code >= UTF8_SURROGATE_LO && _code <= UTF8_SURROGATE_HI )
    {
        return UTF8_UNKNOWN;
    }
    else if( _code < 0x10000 )
    {
        return 3;
    }
    else if( _code <= UTF8_MAX_CODE_POINT )
    {
        return 4;
    }

    return UTF8_UNKNOWN;
}
//////////////////////////////////////////////////////////////////////////
static const wchar_t * __unicode_next_code( const wchar_t * _unicode, const wchar_t * _unicodeEnd, uint32_t * const _unicodeCode )
{
    uint32_t code = (uint32_t)*_unicode++;

    if( UTF8_WCHAR_SIZE == 2 && code >= UTF8_SURROGATE_LO && code <= UTF8_SURROGATE_LEAD_HI )
    {
        // _unicodeEnd is NULL for a NUL-terminated string: L'\0' is never a trail surrogate
        if( _unicode == _unicodeEnd )
        {
            return NULL;
        }

        uint32_t trail = (uint32_t)*_unicode++;

        if( trail < UTF8_SURROGATE_TRAIL_LO || trail > UTF8_SURROGATE_HI )
        {
            return NULL;
        }

        code = 0x10000 + ((code - UTF8_SURROGATE_LO) << 10) + (trail - UTF8_SURROGATE_TRAIL_LO);
    }

    *_unicodeCode = code;

    return _unicode;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_unicodez_size( const wchar_t * _unicode, size_t _unicodeSize )
{
    size_t utf8Size = 0;

    for( const wchar_t
        * it = _unicode,
        *it_end = _unicode + _unicodeSize;
        it != it_end; )
    {
        uint32_t code;
        const wchar_t * it_next = __unicode_next_code( it, it_end, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __utf8_code_size( code );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        utf8Size += codeSize;
    }

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_unicode_size( const wchar_t * _unicode )
{
    size_t utf8Size = 0;

    for( const wchar_t * it = _unicode; *it != L'\0'; )
    {
        uint32_t code;
        const wchar_t * it_next = __unicode_next_code( it, NULL, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __utf8_code_size( code );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        utf8Size += codeSize;
    }

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
static size_t __convert_unicode_to_utf8( uint32_t _code, char * const _utf8, size_t _utf8Size, size_t _utf8Capacity )
{
    if( _code < 0x80 )
    {
        if( _utf8Size + 0 >= _utf8Capacity )
        {
            return UTF8_UNKNOWN;
        }

        _utf8[_utf8Size + 0] = (char)_code;

        return 1;
    }
    else if( _code < 0x800 )
    {
        if( _utf8Size + 1 >= _utf8Capacity )
        {
            return UTF8_UNKNOWN;
        }

        _utf8[_utf8Size + 0] = (char)(0xC0 | (_code >> 6));
        _utf8[_utf8Size + 1] = (char)(0x80 | (_code & 0x3F));

        return 2;
    }
    else if( _code >= UTF8_SURROGATE_LO && _code <= UTF8_SURROGATE_HI )
    {
        return UTF8_UNKNOWN;
    }
    else if( _code < 0x10000 )
    {
        if( _utf8Size + 2 >= _utf8Capacity )
        {
            return UTF8_UNKNOWN;
        }

        _utf8[_utf8Size + 0] = (char)(0xE0 | (_code >> 12));
        _utf8[_utf8Size + 1] = (char)(0x80 | ((_code >> 6) & 0x3F));
        _utf8[_utf8Size + 2] = (char)(0x80 | (_code & 0x3F));

        return 3;
    }
    else if( _code <= UTF8_MAX_CODE_POINT )
    {
        if( _utf8Size + 3 >= _utf8Capacity )
        {
            return UTF8_UNKNOWN;
        }

        _utf8[_utf8Size + 0] = (char)(0xF0 | (_code >> 18));
        _utf8[_utf8Size + 1] = (char)(0x80 | ((_code >> 12) & 0x3F));
        _utf8[_utf8Size + 2] = (char)(0x80 | ((_code >> 6) & 0x3F));
        _utf8[_utf8Size + 3] = (char)(0x80 | (_code & 0x3F));

        return 4;
    }

    return UTF8_UNKNOWN;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_unicodez( const wchar_t * _unicode, size_t _unicodeSize, char * const _utf8, size_t _utf8Capacity )
{
    size_t utf8Size = 0;

    for( const wchar_t
        * it = _unicode,
        *it_end = _unicode + _unicodeSize;
        it != it_end; )
    {
        uint32_t code;
        const wchar_t * it_next = __unicode_next_code( it, it_end, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __convert_unicode_to_utf8( code, _utf8, utf8Size, _utf8Capacity );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        utf8Size += codeSize;
    }

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_unicode( const wchar_t * _unicode, char * const _utf8, size_t _utf8Capacity )
{
    size_t utf8Size = 0;

    for( const wchar_t * it = _unicode; *it != L'\0'; )
    {
        uint32_t code;
        const wchar_t * it_next = __unicode_next_code( it, NULL, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __convert_unicode_to_utf8( code, _utf8, utf8Size, _utf8Capacity );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        utf8Size += codeSize;
    }

    return utf8Size;
}
//////////////////////////////////////////////////////////////////////////
static size_t __unicode_code_size( uint32_t _code )
{
    if( UTF8_WCHAR_SIZE == 2 && _code >= 0x10000 )
    {
        return 2;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////
static const char * __utf8_next_code( const char * _utf8, const char * _utf8End, uint32_t * const _utf8Code )
{
    const uint8_t * p = (const uint8_t *)_utf8;
    const uint8_t * ep = (const uint8_t *)_utf8End;

    uint32_t code;
    size_t codeSize;

    uint32_t b0 = (uint32_t)*p++;

    if( (b0 & 0x80) == 0 )
    {
        code = b0;
        codeSize = 1;
    }
    else if( (b0 & 0xE0) == 0xC0 )
    {
        code = b0 & 0x1F;
        codeSize = 2;
    }
    else if( (b0 & 0xF0) == 0xE0 )
    {
        code = b0 & 0x0F;
        codeSize = 3;
    }
    else if( (b0 & 0xF8) == 0xF0 )
    {
        code = b0 & 0x07;
        codeSize = 4;
    }
    else
    {
        return NULL;
    }

    for( size_t index = 1; index != codeSize; ++index )
    {
        // _utf8End is NULL for a NUL-terminated string: '\0' is never a continuation byte
        if( p == ep )
        {
            return NULL;
        }

        uint32_t trail = (uint32_t)*p++;

        if( (trail & 0xC0) != 0x80 )
        {
            return NULL;
        }

        code = (code << 6) | (trail & 0x3F);
    }

    size_t encodedSize = __utf8_code_size( code );

    if( encodedSize != codeSize )
    {
        return NULL;
    }

    if( _utf8Code != NULL )
    {
        *_utf8Code = code;
    }

    return (const char *)p;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_unicodez_size( const char * _utf8, size_t _utf8Size )
{
    size_t unicodeSize = 0;

    for( const char
        * it = _utf8,
        *it_end = _utf8 + _utf8Size;
        it != it_end; )
    {
        uint32_t code;
        const char * it_next = __utf8_next_code( it, it_end, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __unicode_code_size( code );

        it = it_next;

        unicodeSize += codeSize;
    }

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_unicode_size( const char * _utf8 )
{
    size_t unicodeSize = 0;

    for( const char * it = _utf8; *it != '\0'; )
    {
        uint32_t code;
        const char * it_next = __utf8_next_code( it, NULL, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __unicode_code_size( code );

        it = it_next;

        unicodeSize += codeSize;
    }

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
static size_t __convert_code_to_unicode( uint32_t _code, wchar_t * const _unicode, size_t _unicodeSize, size_t _unicodeCapacity )
{
    if( UTF8_WCHAR_SIZE == 2 && _code >= 0x10000 )
    {
        if( _unicodeSize + 1 >= _unicodeCapacity )
        {
            return UTF8_UNKNOWN;
        }

        uint32_t offset = _code - 0x10000;

        _unicode[_unicodeSize + 0] = (wchar_t)(UTF8_SURROGATE_LO + (offset >> 10));
        _unicode[_unicodeSize + 1] = (wchar_t)(UTF8_SURROGATE_TRAIL_LO + (offset & 0x3FF));

        return 2;
    }

    if( _unicodeSize + 0 >= _unicodeCapacity )
    {
        return UTF8_UNKNOWN;
    }

    _unicode[_unicodeSize + 0] = (wchar_t)_code;

    return 1;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_unicodez( const char * _utf8, size_t _utf8Size, wchar_t * const _unicode, size_t _unicodeCapacity )
{
    size_t unicodeSize = 0;

    for( const char
        * it = _utf8,
        *it_end = _utf8 + _utf8Size;
        it != it_end; )
    {
        uint32_t code;
        const char * it_next = __utf8_next_code( it, it_end, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __convert_code_to_unicode( code, _unicode, unicodeSize, _unicodeCapacity );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        unicodeSize += codeSize;
    }

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_to_unicode( const char * _utf8, wchar_t * const _unicode, size_t _unicodeCapacity )
{
    size_t unicodeSize = 0;

    for( const char * it = _utf8; *it != '\0'; )
    {
        uint32_t code;
        const char * it_next = __utf8_next_code( it, NULL, &code );

        if( it_next == NULL )
        {
            return UTF8_UNKNOWN;
        }

        size_t codeSize = __convert_code_to_unicode( code, _unicode, unicodeSize, _unicodeCapacity );

        if( codeSize == UTF8_UNKNOWN )
        {
            return UTF8_UNKNOWN;
        }

        it = it_next;

        unicodeSize += codeSize;
    }

    return unicodeSize;
}
//////////////////////////////////////////////////////////////////////////
size_t utf8_from_unicode32_symbol( uint32_t _code, char * const _utf8 )
{
    if( _code <= 0x7F )
    {
        _utf8[0] = (char)_code;

        return 1;
    }
    else if( _code <= 0x7FF )
    {
        _utf8[0] = (char)(0xC0 | ((_code >> 6) & 0x1F));
        _utf8[1] = (char)(0x80 | (_code & 0x3F));

        return 2;
    }
    else if( _code >= UTF8_SURROGATE_LO && _code <= UTF8_SURROGATE_HI )
    {
        return UTF8_UNKNOWN;
    }
    else if( _code <= 0xFFFF )
    {
        _utf8[0] = (char)(0xE0 | ((_code >> 12) & 0x0F));
        _utf8[1] = (char)(0x80 | ((_code >> 6) & 0x3F));
        _utf8[2] = (char)(0x80 | (_code & 0x3F));

        return 3;
    }
    else if( _code <= UTF8_MAX_CODE_POINT )
    {
        _utf8[0] = (char)(0xF0 | ((_code >> 18) & 0x07));
        _utf8[1] = (char)(0x80 | ((_code >> 12) & 0x3F));
        _utf8[2] = (char)(0x80 | ((_code >> 6) & 0x3F));
        _utf8[3] = (char)(0x80 | (_code & 0x3F));

        return 4;
    }

    return UTF8_UNKNOWN;
}
//////////////////////////////////////////////////////////////////////////
const char * utf8_next_code( const char * _utf8, const char * _utf8End, uint32_t * const _utf8Code )
{
    if( _utf8 == _utf8End )
    {
        return NULL;
    }

    const char * utf8Next = __utf8_next_code( _utf8, _utf8End, _utf8Code );

    return utf8Next;
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
const char * utf8_replace_invalid( const char * _utf8, const char * _utf8End, char * const _utf8Out )
{
    char * uft8Work = _utf8Out;

    for( const char * p = _utf8; p != _utf8End; )
    {
        uint32_t code;
        const char * next = utf8_next_code( p, _utf8End, &code );

        if( next != NULL )
        {
            uft8Work = __append_code_point( uft8Work, code );

            p = next;
        }
        else
        {
            uft8Work = __append_code_point( uft8Work, UTF8_REPLACEMENT_CHARACTER );

            ++p;
        }
    }

    return uft8Work;
}
//////////////////////////////////////////////////////////////////////////