/*

  Copyright (C) 2003 - 2014  Razvan Cojocaru <rzvncj@gmail.com>
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


#include <wx/string.h>
#include <stdint.h>
#include <memory>


#define FIXENDIAN16(x) (x = wxUINT16_SWAP_ON_BE(x))
#define FIXENDIAN32(x) (x = wxUINT32_SWAP_ON_BE(x))
#define UINT16ARRAY(x) ((unsigned char)(x)[0] | ((uint16_t)(x)[1] << 8))
#define UINT32ARRAY(x) (UINT16ARRAY(x) | ((uint32_t)(x)[2] << 16) \
		| ((uint32_t)(x)[3] << 24))
#define INT32ARRAY(x) ((int32_t)UINT32ARRAY(x))
#define INT16ARRAY(x) ((int16_t)UINT16ARRAY(x))



#if wxUSE_UNICODE

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x), wxConvISO8859_1)

#	define CURRENT_CHAR_STRING_CV(x, cv) \
	wxString(reinterpret_cast<const char *>(x), cv)

#else

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x))

#	define CURRENT_CHAR_STRING_CV(x, cv) \
	wxString(reinterpret_cast<const char *>(x))

#endif


inline uint64_t be_encint(unsigned char* buffer, size_t& length)
{
	uint64_t result = 0;
	int shift=0;
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
	int bits = 0;
	length = 0;

	while(*byte & (1 << *bit)){
		if(*bit)
			--(*bit);
		else {
			++byte;
			++length;
			*bit = 7;
		}
		++bits;
	}

	if(*bit)
		--(*bit);
	else {
		++length;
		*bit = 7;
	}

	return bits;
}


inline uint64_t sr_int(unsigned char* byte, int* bit,
			unsigned char s, unsigned char r, size_t& length)
{
	uint64_t ret;
	unsigned char mask;
	int n, n_bits, num_bits, base, count;
	length = 0;
	size_t fflen;

	if(!bit || *bit > 7 || s != 2)
		return ~(uint64_t)0;
	ret = 0;

	count = ffus(byte, bit, fflen);
	length += fflen;
	byte += length;

	n_bits = n = r + (count ? count-1 : 0) ;

	while(n > 0) {
		num_bits = n > *bit ? *bit : n-1;
		base = n > *bit ? 0 : *bit - (n-1);

		switch(num_bits){
			case 0:
				mask = 1;
				break;
			case 1:
				mask = 3;
				break;
			case 2:
				mask = 7;
				break;
			case 3:
				mask = 0xf;
				break;
			case 4:
				mask = 0x1f;
				break;
			case 5:
				mask = 0x3f;
				break;
			case 6:
				mask = 0x7f;
				break;
			case 7:
				mask = 0xff;
				break;
                        default:
                                mask = 0xff;
				break;
		}

		mask <<= base;
		ret = (ret << (num_bits+1)) |
			(uint64_t)((*byte & mask) >> base);

		if( n > *bit ){
			++byte;
			++length;
			n -= *bit+1;
			*bit = 7;
		} else {
			*bit -= n;
			n = 0;
		}
	}

	if(count)
		ret |= (uint64_t)1 << n_bits;

	return ret;
}


#if wxUSE_UNICODE
#	define UNICODE_PARAM(x) x
#	define NON_UNICODE_PARAM(x)
#else
#	define UNICODE_PARAM(x)
#	define NON_UNICODE_PARAM(x) x
#endif


inline void createCSConvPtr(std::auto_ptr<wxCSConv>& cvPtr, wxFontEncoding enc)
{
#ifdef __WXMSW__
		cvPtr.reset(new wxCSConv(enc));
#else
		switch(enc) {
		case wxFONTENCODING_CP950:
			cvPtr.reset(new wxCSConv(wxT("BIG5")));
			break;
		case wxFONTENCODING_CP932:
			cvPtr.reset(new wxCSConv(wxT("SJIS")));
			break;
		default:
			cvPtr.reset(new wxCSConv(enc));
			break;
		}
#endif
}


#if wxUSE_UNICODE

inline wxString translateEncoding(const wxString& input, wxFontEncoding enc)
{
        if(input.IsEmpty())
                return wxEmptyString;

        if(enc != wxFONTENCODING_SYSTEM) {
		wxCSConv convFrom(wxFONTENCODING_ISO8859_1);

		std::auto_ptr<wxCSConv> convToPtr;
		createCSConvPtr(convToPtr, enc);

		return wxString(input.mb_str(convFrom), *convToPtr);
	}

        return input;
}
#else
#define translateEncoding(x, y) x
#endif


inline wxChar charForCode(unsigned code,
			  const wxCSConv& NON_UNICODE_PARAM(cv),
			  bool NON_UNICODE_PARAM(conv))
{
#if !wxUSE_UNICODE

#	if wxUSE_WCHAR_T

	if(code < 256)
		return (wxChar)code;

	if(conv) {
		char buf[2];
		wchar_t wbuf[2];
		wbuf[0] = (wchar_t)code;
		wbuf[1] = 0;

		cv.WC2MB(buf, wbuf, 2);

		if(cv.WC2MB(buf, wbuf, 2) == (size_t)-1)
			return '?';

		return buf[0];
	} else
		return (wxChar)code;

#	else
	return (code < 256) ? (wxChar)code : '?';

#	endif

#else
	return (wxChar)code;
#endif
}


/*
  Local Variables:
  mode: c++
  c-basic-offset: 8
  tab-width: 8
  c-indent-comments-syntactically-p: t
  c-tab-always-indent: t
  indent-tabs-mode: t
  End:
*/

// vim:shiftwidth=8:autoindent:tabstop=8:noexpandtab:softtabstop=8

