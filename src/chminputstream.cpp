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


#include <chminputstream.h>


/*----------------------------------------------------------------------
 * class CHMInputStream static members
 */


CHMFile *CHMInputStream::_archiveCache = NULL;
wxString CHMInputStream::_path;
wxMutex CHMInputStream::_mutex;


void CHMInputStream::Cleanup()
{
	wxMutexLocker lock(_mutex);

	if(_archiveCache != NULL) {
		delete _archiveCache;
		_archiveCache = NULL;
	}
}


/*----------------------------------------------------------------------
 * rest of class CHMInputStream implementation
 */

CHMInputStream::CHMInputStream(const wxString& archive, 
			       const wxString& file)
	: _currPos(0)
{
	wxString filename = file;

	_mutex.Lock();

	if(!archive.IsEmpty())
		_path = archive.BeforeLast(wxT('/')) + wxT("/");

	memset(&_ui, 0, sizeof(_ui));

	_mutex.Unlock();

	// Maybe the cached chmFile* isn't valid anymore,
	// or maybe there is no chached chmFile* yet.
	if(!archive.IsEmpty() && !Init(archive)) {
		m_lasterror = wxSTREAM_READ_ERROR;
		return;
	}

	_mutex.Lock();
	
	// Somebody's looking for the homepage.
	if(file.IsSameAs(wxT("/")))
		filename = _archiveCache->HomePage();

	if(!filename.Left(8).CmpNoCase(wxT("/MS-ITS:"))) {
		// If this ever happens chances are Microsoft
		// decided that even if we went through the
		// trouble to open this archive and check out
		// the index file, the index file is just a
		// link to a file in another archive.

		wxString arch_link = 
			filename.AfterFirst(wxT(':')).BeforeFirst(wxT(':'));

		filename = filename.AfterLast(wxT(':'));

		_mutex.Unlock();

		// Reset the cached chmFile* and all.
		if(!Init(arch_link))
			if(!Init(_path + arch_link)) {
				m_lasterror = wxSTREAM_READ_ERROR;
				return;
			}

		_mutex.Lock();
	}

	assert(_archiveCache != NULL);

	// See if the file really is in the archive.
	if(!_archiveCache->ResolveObject(filename, &_ui)) {
		m_lasterror = wxSTREAM_READ_ERROR;
		_mutex.Unlock();
		return;
	}

	_mutex.Unlock();
}


size_t CHMInputStream::GetSize() const
{
	wxMutexLocker lock(_mutex);
	return _ui.length;
}


bool CHMInputStream::Eof() const
{
	wxMutexLocker lock(_mutex);
	return _currPos >= (off_t)_ui.length;
}


size_t CHMInputStream::OnSysRead(void *buffer, size_t bufsize)
{	
	wxMutexLocker lock(_mutex);

	if(_currPos >= (off_t)_ui.length) {
		m_lasterror = wxSTREAM_EOF;
		return 0;
	}

	if(!_archiveCache)
		return 0;

	if(_currPos + bufsize > (off_t)_ui.length)
        bufsize = _ui.length - _currPos;
        
	bufsize = 
		_archiveCache->RetrieveObject(&_ui,
					      (unsigned char *)buffer, 
					      _currPos, bufsize);
     
	_currPos += bufsize;
    
	return bufsize;
}


off_t CHMInputStream::OnSysSeek(off_t seek, wxSeekMode mode)
{
	wxMutexLocker lock(_mutex);

	switch(mode) {
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
	_mutex.Lock();

	if(_archiveCache == NULL || 
	   !_archiveCache->ArchiveName().IsSameAs(archive)) {
	 
		_mutex.Unlock();
		Cleanup();
		_mutex.Lock();

		_archiveCache = new CHMFile(archive);

		if(!_archiveCache->IsOk()) {
			_mutex.Unlock();
			Cleanup();
			return false;
		}
	}

	_mutex.Unlock();

	return true;
}
