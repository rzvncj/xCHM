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


#ifndef __CHMINPUTSTREAM_H_
#define __CHMINPUTSTREAM_H_

#include <wx/stream.h>
#include <chmfile.h>


class CHMInputStream : public wxInputStream
{
public:
	CHMInputStream(const wxString& archive, const wxString& file);

	virtual size_t GetSize() const;
	virtual bool Eof() const;

	static CHMFile* GetCache() { return _archiveCache; }
	static void Cleanup();
	
protected:
	virtual size_t OnSysRead(void *buffer, size_t bufsize);
	virtual off_t OnSysSeek(off_t seek, wxSeekMode mode);
	virtual off_t OnSysTell() const { return _currPos; }

private:
	bool Init(const wxString& archive);


private:
	// Obviously thread-unsafe. :)
	static CHMFile *_archiveCache;
	off_t _currPos;
	chmUnitInfo _ui;
};

#endif // __CHMINPUTSTREAM_H_

