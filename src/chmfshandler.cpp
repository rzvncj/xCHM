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


#include <chmfshandler.h>
#include <chminputstream.h>


// only needs to be here because I killed the constructors
// by providing a private copy constructor
CHMFSHandler::CHMFSHandler()
{
}


CHMFSHandler::~CHMFSHandler()
{
	CHMInputStream::Cleanup();
}


bool CHMFSHandler::CanOpen(const wxString& location)
{
	wxString p = GetProtocol(location);
	return (p == wxT("chm")) &&
		(GetProtocol(GetLeftLocation(location)) == wxT("file"));
}


wxFSFile* CHMFSHandler::OpenFile(wxFileSystem& WXUNUSED(fs), 
				 const wxString& location)
{
	wxString right = GetRightLocation(location);
	wxString left = GetLeftLocation(location);
	CHMInputStream *s;
	
	// HTML code for space is %20
	right.Replace("%20", " ", TRUE);

	if (GetProtocol(left) != wxT("file"))
		return NULL;
            
	wxFileName leftFilename = wxFileSystem::URLToFileName(left);    
	s = new CHMInputStream(leftFilename.GetFullPath(), right);

	if (s && s->IsOk()) {
		
		if(right.IsSameAs("/"))
			right = s->GetCache()->HomePage();

		// The dreaded links to files in other archives.
		// Talk about too much enthusiasm.
		if(right.StartsWith("/MS-ITS:"))
			right = right.AfterLast(':');

		return new wxFSFile(s,
				    wxString("file:") +
				    s->GetCache()->ArchiveName() + 
				    wxT("#chm:") + right,
				    GetMimeTypeFromExt(right),
				    GetAnchor(location),
				    wxDateTime(wxFileModificationTime(left)));
	}
    
	delete s;
	return NULL;
}
