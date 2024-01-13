/*
  Copyright (C) 2003 - 2024  Razvan Cojocaru <rzvncj@gmail.com>

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

#include <chminputstream.h>

/*----------------------------------------------------------------------
 * class CHMInputStream static members
 */

std::unique_ptr<CHMFile> CHMInputStream::_archiveCache;
wxString                 CHMInputStream::_path;

void CHMInputStream::Cleanup()
{
    _archiveCache.reset();
}

CHMFile* CHMInputStream::GetCache()
{
    return _archiveCache.get();
}

/*----------------------------------------------------------------------
 * rest of class CHMInputStream implementation
 */

CHMInputStream::CHMInputStream(const wxString& archive, const wxString& file)
{
    auto filename = file;

    if (!archive.IsEmpty())
        _path = archive.BeforeLast(wxT('/')) + wxT("/");

    // Maybe the cached chmFile* isn't valid anymore, or maybe there is no chached chmFile* yet.
    if (!archive.IsEmpty() && !Init(archive)) {
        m_lasterror = wxSTREAM_READ_ERROR;
        return;
    }

    assert(_archiveCache);

    // Somebody's looking for the homepage.
    if (file.IsSameAs(wxT("/")))
        filename = _archiveCache->HomePage();

    if (!filename.Left(8).CmpNoCase(wxT("/MS-ITS:"))) {
        // If this ever happens chances are Microsoft decided that even if we went through the
        // trouble to open this archive and check out the index file, the index file is just a
        // link to a file in another archive.

        auto arch_link = filename.AfterFirst(wxT(':')).BeforeFirst(wxT(':'));

        filename = filename.AfterLast(wxT(':'));

        // Reset the cached chmFile* and all.
        if (!Init(arch_link))
            if (!Init(_path + arch_link)) {
                m_lasterror = wxSTREAM_READ_ERROR;
                return;
            }
    }

    // See if the file really is in the archive.
    if (!_archiveCache->ResolveObject(filename, &_ui)) {
        m_lasterror = wxSTREAM_READ_ERROR;
        return;
    }
}

size_t CHMInputStream::GetSize() const
{
    return _ui.length;
}

bool CHMInputStream::Eof() const
{
    return static_cast<uint64_t>(_currPos) >= _ui.length;
}

size_t CHMInputStream::OnSysRead(void* buffer, size_t bufsize)
{
    if (static_cast<uint64_t>(_currPos) >= _ui.length) {
        m_lasterror = wxSTREAM_EOF;
        return 0;
    }

    if (!_archiveCache)
        return 0;

    if (static_cast<uint64_t>(_currPos + bufsize) > _ui.length)
        bufsize = _ui.length - _currPos;

    bufsize = _archiveCache->RetrieveObject(&_ui, static_cast<unsigned char*>(buffer), _currPos, bufsize);
    _currPos += bufsize;

    return bufsize;
}

wxFileOffset CHMInputStream::OnSysSeek(wxFileOffset seek, wxSeekMode mode)
{
    switch (mode) {
    case wxFromCurrent:
        _currPos += seek;
        break;
    case wxFromStart:
        _currPos = seek;
        break;
    case wxFromEnd:
        _currPos = _ui.length - 1 + seek;
        break;
    default:
        _currPos = seek;
    }

    return _currPos;
}

bool CHMInputStream::Init(const wxString& archive)
{
    if (!_archiveCache || !_archiveCache->ArchiveName().IsSameAs(archive)) {
        _archiveCache = std::make_unique<CHMFile>(archive);

        if (!_archiveCache->IsOk()) {
            Cleanup();
            return false;
        }
    }

    return true;
}
