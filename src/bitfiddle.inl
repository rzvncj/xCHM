/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
 
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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#define FIXENDIAN16(x) (x = wxUINT16_SWAP_ON_BE(x))
#define FIXENDIAN32(x) (x = wxUINT32_SWAP_ON_BE(x))


inline u_int64_t be_encint(unsigned char* buffer, size_t& length)
{
	u_int64_t result = 0;
	int shift=0;
	length = 0;

	do {
		result |= ((*buffer) & 0x7f) << shift;
		shift += 7;
		++length;

	} while (*(buffer++) & 0x80);
	
	return result;
}
