/*

  Copyright (C) 2003 - 2014  Razvan Cojocaru <rzvncj@gmail.com>
 
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
	return (p == wxT("xchm") 
		&& GetProtocol(GetLeftLocation(location)) == wxT("file"))
		|| !location.Left(6).CmpNoCase(wxT("MS-ITS"));
}


wxFSFile* CHMFSHandler::OpenFile(wxFileSystem& fs, 
				 const wxString& location)
{
	wxString right = GetRightLocation(location);
	wxString left = GetLeftLocation(location);
	wxString cwd = GetRightLocation(fs.GetPath());
	CHMInputStream *s = NULL;

	if(!location.Left(6).CmpNoCase(wxT("MS-ITS"))) {
		right = wxString(wxT("/")) + location;
		left = wxEmptyString;

	} else if (GetProtocol(left) != wxT("file"))
		return NULL;

	// HTML code for space is %20
	right.Replace(wxT("%20"), wxT(" "), TRUE);
	right.Replace(wxT("%5F"), wxT("_"), TRUE);
	right.Replace(wxT("%2E"), wxT("."), TRUE);
	right.Replace(wxT("%2D"), wxT("-"), TRUE);
	right.Replace(wxT("%26"), wxT("&"), TRUE);
            
	wxFileName filename = wxFileSystem::URLToFileName(left);
	filename.Normalize();

	size_t len = cwd.Length();
	if(right.Length() > len && right.StartsWith(cwd)
			&& right[len] == wxT('/'))
		right = right.Mid(len);

        wxFileName fwfn(right, wxPATH_UNIX);
        fwfn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE, cwd, 
			wxPATH_UNIX);
	right = fwfn.GetFullPath(wxPATH_UNIX);

	s = new CHMInputStream(left.IsEmpty() ? 
			       left : filename.GetFullPath(), right);

	if (s && s->IsOk()) {

		if(right.IsSameAs(wxT("/")))
			right = s->GetCache()->HomePage();

		// The dreaded links to files in other archives.
		// Talk about too much enthusiasm.
		if(!right.Left(8).CmpNoCase(wxT("/MS-ITS:")))
			right = right.AfterLast(wxT(':'));

		return new wxFSFile(s, 
				    wxString(wxT("file:")) +
				    s->GetCache()->ArchiveName() + 
				    wxT("#xchm:") + right, 
				    GetMimeTypeFromExt(right.Lower()),
				    GetAnchor(location),
				    wxDateTime((time_t)-1));
	}
    
	delete s;
	return NULL;
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
