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


#ifndef __CHMFILE_H_
#define __CHMFILE_H_

#include <chm_lib.h>
#include <wx/string.h>
#include <wx/treectrl.h>


class CHMFile {
public:
	CHMFile();
	CHMFile(const wxString& archiveName);
	~CHMFile();

	wxString HomePage() { return _home; }
	wxString TopicsFile() { return _topicsFile; }
	wxString ArchiveName() { return _filename; }
	wxString IndexFile() { return _indexFile; }
	wxString Title() { return _title; }

	// Trying really hard to refrain from using exceptions
	// as that seems to be wxWindows' philosophy.
	bool IsOk() { return _chmFile != NULL; }

	bool LoadCHM(const wxString& archiveName);
	void CloseCHM();

	bool GetTopicsTree(wxTreeCtrl *toBuild);

	bool ResolveObject(const wxString& fileName, chmUnitInfo *ui);
	LONGINT64 RetrieveObject(chmUnitInfo *ui, unsigned char *buffer,
				 LONGUINT64 fileOffset, LONGINT64 bufferSize);

private:
	bool GetArchiveInfo();

private:
	chmFile* _chmFile;
	wxString _filename;
	wxString _home;
	wxString _topicsFile;
	wxString _indexFile;
	wxString _title;
private:
	CHMFile(const CHMFile&);
	CHMFile& operator=(const CHMFile&);
};


#endif // __CHMFILE_H_
