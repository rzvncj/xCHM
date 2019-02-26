/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>
  Most of the code in this file is a modified version of code from
  Pabs' GPL chmdeco project, credits and thanks go to him.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.
*/

#include <memory>
#include <stdint.h>
#include <wx/string.h>

#define FIXENDIAN16(x) (x = wxUINT16_SWAP_ON_BE(x))
#define FIXENDIAN32(x) (x = wxUINT32_SWAP_ON_BE(x))
#define UINT16ARRAY(x) ((unsigned char)(x)[0] | ((uint16_t)(x)[1] << 8))
#define UINT32ARRAY(x) (UINT16ARRAY(x) | ((uint32_t)(x)[2] << 16) | ((uint32_t)(x)[3] << 24))
#define INT32ARRAY(x) ((int32_t)UINT32ARRAY(x))
#define INT16ARRAY(x) ((int16_t)UINT16ARRAY(x))

#if wxUSE_UNICODE
#define CURRENT_CHAR_STRING(x) wxString(reinterpret_cast<const char*>(x), wxConvISO8859_1)
#define CURRENT_CHAR_STRING_CV(x, cv) wxString(reinterpret_cast<const char*>(x), cv)
#else
#define CURRENT_CHAR_STRING(x) wxString(reinterpret_cast<const char*>(x))
#define CURRENT_CHAR_STRING_CV(x, cv) wxString(reinterpret_cast<const char*>(x))
#endif

inline uint64_t be_encint(unsigned char* buffer, size_t& length)
{
    uint64_t result {0};
    int      shift {0};

    length = 0;

    do {
        result |= ((*buffer) & 0x7f) << shift;
        shift += 7;
        ++length;

    } while (*(buffer++) & 0x80);

    return result;
}

/*
   Finds the first unset bit in memory. Returns the number of set bits found.
   Returns -1 if the buffer runs out before we find an unset bit.
*/
inline int ffus(unsigned char* byte, int* bit, size_t& length)
{
    int bits {0};

    length = 0;

    while (*byte & (1 << *bit)) {
        if (*bit)
            --(*bit);
        else {
            ++byte;
            ++length;
            *bit = 7;
        }
        ++bits;
    }

    if (*bit)
        --(*bit);
    else {
        ++length;
        *bit = 7;
    }

    return bits;
}

inline uint64_t sr_int(unsigned char* byte, int* bit, unsigned char s, unsigned char r, size_t& length)
{
    uint64_t      ret {0};
    unsigned char mask;
    int           n, n_bits, num_bits, base, count;
    size_t        fflen;

    length = 0;

    if (!bit || *bit > 7 || s != 2)
        return ~static_cast<uint64_t>(0);

    count = ffus(byte, bit, fflen);
    length += fflen;
    byte += length;

    n_bits = n = r + (count ? count - 1 : 0);

    while (n > 0) {
        num_bits = n > *bit ? *bit : n - 1;
        base     = n > *bit ? 0 : *bit - (n - 1);
        mask     = (0xff >> (7 - num_bits)) << base;
        ret      = (ret << (num_bits + 1)) | static_cast<uint64_t>((*byte & mask) >> base);

        if (n > *bit) {
            ++byte;
            ++length;
            n -= *bit + 1;
            *bit = 7;
        } else {
            *bit -= n;
            n = 0;
        }
    }

    if (count)
        ret |= static_cast<uint64_t>(1) << n_bits;

    return ret;
}

#if wxUSE_UNICODE
#define UNICODE_PARAM(x) x
#define NON_UNICODE_PARAM(x)
#else
#define UNICODE_PARAM(x)
#define NON_UNICODE_PARAM(x) x
#endif

inline std::unique_ptr<wxCSConv> createCSConvPtr(wxFontEncoding enc)
{
#ifdef __WXMSW__
    return std::make_unique<wxCSConv>(enc);
#else
    switch (enc) {
    case wxFONTENCODING_CP950:
        return std::make_unique<wxCSConv>(wxT("BIG5"));
    case wxFONTENCODING_CP932:
        return std::make_unique<wxCSConv>(wxT("SJIS"));
    default:
        return std::make_unique<wxCSConv>(enc);
    }
#endif
}

#if wxUSE_UNICODE
inline wxString translateEncoding(const wxString& input, wxFontEncoding enc)
{
    if (!input.IsEmpty() && enc != wxFONTENCODING_SYSTEM) {
        wxCSConv                  convFrom(wxFONTENCODING_ISO8859_1);
        std::unique_ptr<wxCSConv> convToPtr = createCSConvPtr(enc);

        return wxString(input.mb_str(convFrom), *convToPtr);
    }

    return input;
}
#else
#define translateEncoding(x, y) x
#endif

inline wxChar charForCode(unsigned code, const wxCSConv& NON_UNICODE_PARAM(cv), bool NON_UNICODE_PARAM(conv))
{
#if !wxUSE_UNICODE
#if wxUSE_WCHAR_T

    if (code < 256)
        return static_cast<wxChar>(code);

    if (conv) {
        char    buf[2];
        wchar_t wbuf[2] {};

        wbuf[0] = static_cast<wchar_t>(code);

        cv.WC2MB(buf, wbuf, 2);

        if (cv.WC2MB(buf, wbuf, 2) == static_cast<size_t>(-1))
            return '?';

        return buf[0];
    } else
        return static_cast<wxChar>(code);

#else
    return (code < 256) ? static_cast<wxChar>(code) : '?';
#endif
#else
    return static_cast<wxChar>(code);
#endif
}
