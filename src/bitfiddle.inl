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

inline uint16_t UINT16ARRAY(const unsigned char* x)
{
    return x[0] | (static_cast<uint16_t>(x[1]) << 8);
}

inline uint32_t UINT32ARRAY(const unsigned char* x)
{
    return UINT16ARRAY(x) | (static_cast<uint32_t>(x[2]) << 16) | (static_cast<uint32_t>(x[3]) << 24);
}

#define INT32ARRAY(x) static_cast<int32_t>(UINT32ARRAY(x))
#define INT16ARRAY(x) static_cast<int16_t>(UINT16ARRAY(x))

#if wxUSE_UNICODE
#define CURRENT_CHAR_STRING(x) wxString(reinterpret_cast<const char*>(x), wxConvISO8859_1)
#define CURRENT_CHAR_STRING_CV(x, cv) wxString(reinterpret_cast<const char*>(x), cv)
#else
#define CURRENT_CHAR_STRING(x) wxString(reinterpret_cast<const char*>(x))
#define CURRENT_CHAR_STRING_CV(x, cv) wxString(reinterpret_cast<const char*>(x))
#endif

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
