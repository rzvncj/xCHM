/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>

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

CHMFSHandler::~CHMFSHandler()
{
    CHMInputStream::Cleanup();
}

bool CHMFSHandler::CanOpen(const wxString& location)
{
    auto p = GetProtocol(location);
    return (p == wxT("xchm") && GetProtocol(GetLeftLocation(location)) == wxT("file"))
        || !location.Left(6).CmpNoCase(wxT("MS-ITS"));
}

wxFSFile* CHMFSHandler::OpenFile(wxFileSystem& fs, const wxString& location)
{
    auto right = GetRightLocation(location);
    auto left  = GetLeftLocation(location);
    auto cwd   = GetRightLocation(fs.GetPath());

    if (!location.Left(6).CmpNoCase(wxT("MS-ITS"))) {
        right = wxT("/") + location;
        left  = wxEmptyString;

    } else if (GetProtocol(left) != wxT("file"))
        return nullptr;

    // HTML code for space is %20
    right.Replace(wxT("%20"), wxT(" "), true);
    right.Replace(wxT("%5F"), wxT("_"), true);
    right.Replace(wxT("%2E"), wxT("."), true);
    right.Replace(wxT("%2D"), wxT("-"), true);
    right.Replace(wxT("%26"), wxT("&"), true);

    auto filename = wxFileSystem::URLToFileName(left);
    filename.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE
                       | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);

    auto len = cwd.Length();
    if (right.Length() > len && right.StartsWith(cwd) && right[len] == wxT('/'))
        right = right.Mid(len);

    wxFileName fwfn(right, wxPATH_UNIX);
    fwfn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE, cwd, wxPATH_UNIX);
    right = fwfn.GetFullPath(wxPATH_UNIX);

    auto s = std::make_unique<CHMInputStream>(left.IsEmpty() ? left : filename.GetFullPath(), right);

    if (s && s->IsOk()) {
        auto cache = s->GetCache();

        if (right.IsSameAs(wxT("/")))
            right = cache->HomePage();

        // Links to files in other archives.
        if (!right.Left(8).CmpNoCase(wxT("/MS-ITS:")))
            right = right.AfterLast(wxT(':'));

        auto newLocation = wxT("file:") + cache->ArchiveName() + wxT("#xchm:") + right;

        return new wxFSFile(s.release(), newLocation, GetMimeTypeFromExt(right.Lower()), GetAnchor(location), {});
    }

    return nullptr;
}
