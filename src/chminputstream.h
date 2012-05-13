/*

  Copyright (C) 2003 - 2012  Razvan Cojocaru <razvanco@gmx.net>
 
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


#ifndef __CHMINPUTSTREAM_H_
#define __CHMINPUTSTREAM_H_

#include <wx/stream.h>
#include <chmfile.h>


/*!
  \class wxInputStream
  \brief wxWidgets input stream class.
*/


//! Input stream from a .chm archive.
class CHMInputStream : public wxInputStream
{
public:
	/*!
	  \brief Creates a stream.
	  \param archive The .chm file name on disk. It makes sense to
	  pass the empty string if you're sure file is a link to a
	  page, in the form of "/MS-ITS:archive.chm::/filename.html".
	  \param file The file requested from the archive.
	 */
	CHMInputStream(const wxString& archive, const wxString& file);

	//! Returns the size of the file.
	virtual size_t GetSize() const;

	//! True if EOF has been found.
	virtual bool Eof() const;

	/*!
	  \brief Returns the static CHMFile pointer associated with
	  this stream. Archives are being cached until it is
	  explicitly requested to open a different one.
	  \return A valid pointer to a CHMFile object or NULL if no
	  .chm file has been opened yet.
	 */
	static CHMFile* GetCache();
	/*!
	  \brief Cleans up the cache. Has to be public and static
	  since the stream doesn't know how many other streams using
	  the same cache will be created after it. Somebody else has
	  to turn off the lights, and in this case it's CHMFSHandler.
	 */
	static void Cleanup();
	
protected:
	/*! 
	  \brief Attempts to read a chunk from the stream.
	  \param buffer The read data is being placed here.
	  \param bufsize Number of bytes requested.
	  \return Number of bytes actually read.
	*/
	virtual size_t OnSysRead(void *buffer, size_t bufsize);

	/*!
	  \brief Seeks to the requested position in the file.
	  \param seek Where to seek.
	  \param mode Seek from the beginning, current position, etc.
	  \return Position in the file.
	 */
#ifdef __WXMSW__
	virtual wxFileOffset OnSysSeek(wxFileOffset seek, wxSeekMode mode);
#else
	virtual off_t OnSysSeek(off_t seek, wxSeekMode mode);
#endif

	/*!
	  \brief Asks what is the current position in the file.
	  \return Current position.
	 */
#ifdef __WXMSW__
	virtual wxFileOffset OnSysTell() const { return _currPos; }
#else
	virtual off_t OnSysTell() const { return _currPos; }
#endif

private:
	//! Helper. Inits the cache.
	bool Init(const wxString& archive);


private:
	static CHMFile *_archiveCache;
	off_t _currPos;
	chmUnitInfo _ui;
	static wxString _path;
};

#endif // __CHMINPUTSTREAM_H_


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
