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
#include <wx/listbox.h>
#include <wx/hashmap.h>


//! Declares a class called CHMSearchResults - <string, string> hashmap.
WX_DECLARE_STRING_HASH_MAP(wxString, CHMSearchResults);


//! Mostly a C++ wrapper around the CHMLIB facilities. Concrete class.
class CHMFile {
public:
	//! Default constructor. Doesn't do anthing except some initializations.
	CHMFile();
	
	/*!
	  \brief This constructor attempts to open the .chm file passed as
	  it's argument. If it fails, IsOk() will return false.
	  \param archiveName The .chm filename on disk.
	 */
	CHMFile(const wxString& archiveName);

	//! Destructor. If a file has been succesfully opened, it closes it.
	~CHMFile();

	
	/*!
	  \brief Gets the name of the default page in the archive.
	  \return The home page name, with a '/' added in front and
	  relative to the root of the archive filesystem. If no .chm
	  has been opened, the returned value is "/".
	 */
	wxString HomePage() { return _home; }

	/*!
	  \brief Gets name of the .hhc file in the archive that
	  could potentially be used to generate content information from.
	  \return The topics file name, with a '/' added in front and
	  relative to the root of the archive filesystem. If no .chm
	  has been opened, the return value is an empty string.
	 */
	wxString TopicsFile() { return _topicsFile; }

	/*!
	  \brief Gets the filename of the currently opened .chm file.
	  \return The filename of the currently opened archive, relative
	  to the root of the filesystem, or the empty string if no
	  archive has been opened.
	 */
	wxString ArchiveName() { return _filename; }

	/*!
	  \brief Gets name of the .hhk file in the archive that
	  could potentially be used to generate content information from.
	  \return The index file name, with a '/' added in front and
	  relative to the root of the archive filesystem. If no .chm
	  has been opened, the return value is an empty string.
	 */
	wxString IndexFile() { return _indexFile; }

	/*!
	  \brief Gets the name of the opened .chm.
	  \return The name of the opened document, or an empty string
	  if no .chm has been loaded.
	*/
	wxString Title() { return _title; }

	/*!
	  \brief Checks if the last attempt to load a .chm file was
	  succesful.
	  \return true, if the last attempt to load a .chm file was
	  succesful, false otherwise.
	 */
	bool IsOk() { return _chmFile != NULL; }


	/*!
	  \brief Attempts to load a .chm file from disk.
	  \param archiveName The .chm filename on disk.
	  \return true on success, false on failure.
	 */
	bool LoadCHM(const wxString& archiveName);

	//! Closes the currently opened .chm, or does nothing if none opened.
	void CloseCHM();

	/*!
	  \brief Attempts to create a wxTreeCtrl by parsing the topics
	  file, or if that's not available, the index file.
	  \param toBuild Pointer to the tree that will be constructed.
	  If neither a topics file or an index file are available, the
	  tree is unmodified. The tree must be empty before passing it
	  to this function.
	  \return true if it's possible to build the tree, false otherwise.
	 */
	bool GetTopicsTree(wxTreeCtrl *toBuild);


	/*!
	  \brief Fast search using the $FIftiMain file in the .chm.
	  \param text The text we're looking for.
	  \param wholeWords Are we looking for whole words only?
	  \param titlesOnly Are we looking for titles only?
	  \param results A string-string hashmap that will hold
	  the results in case of successful search. The keys are
	  the URLs and the values are the page titles.
	  \return true if the search succeeded, false otherwise.
	 */
	bool IndexSearch(const wxString& text, bool wholeWords, 
			 bool titlesOnly, CHMSearchResults *results);

	/*!
	  \brief Looks up fileName in the archive.
	  \param fileName The file name to look up in the archive,
	  qualified with '/' standing for the root of the archive
	  filesystem.
	  \param ui A pointer to CHMLIB specific data about the file.
	  The parameter gets filled with useful data if the lookup 
	  was succesful.
	  \return true if the file exists in the archive, false otherwise.
	 */
	bool ResolveObject(const wxString& fileName, chmUnitInfo *ui);

	/*!
	  \brief Retrieves an uncompressed chunk of a file in the .chm.
	  \param ui Pointer to a CHMLIB specific data structure obtained
	  from a succesful call to ResolveObject().
	  \param buffer The buffer to place the chunk into.
	  \param fileOffset Where does the chunk we want begin in the file?
	  \param bufferSize The size of the buffer.
	  \return 0 on error, length of chunk retrieved otherwise.
	 */
	size_t RetrieveObject(chmUnitInfo *ui, unsigned char *buffer,
			      off_t fileOffset, size_t bufferSize);

private:
	//! Helper. Initializes most of the private data members.
	bool GetArchiveInfo();

	//! Helper. Returns the $FIftiMain offset of leaf node or 0.
	u_int32_t GetLeafNodeOffset(const wxString& text,
				    u_int32_t initalOffset,
				    u_int32_t buffSize,
				    u_int16_t treeDepth,
				    chmUnitInfo *ui);

	//! Helper. Processes the word location code entries while searching.
	bool ProcessWLC(u_int64_t wlc_count, u_int64_t wlc_size,
			u_int32_t wlc_offset, unsigned char ds,
			unsigned char dr, unsigned char cs,
			unsigned char cr, unsigned char ls,
			unsigned char lr, chmUnitInfo *uifmain,
			chmUnitInfo* uitbl, chmUnitInfo *uistrings,
			chmUnitInfo* topics, chmUnitInfo *urlstr,
			CHMSearchResults *results);

private:
	chmFile* _chmFile;
	wxString _filename;
	wxString _home;
	wxString _topicsFile;
	wxString _indexFile;
	wxString _title;

private:
	//! No copy construction allowed.
	CHMFile(const CHMFile&);

	//! No assignments.
	CHMFile& operator=(const CHMFile&);
};


#endif // __CHMFILE_H_
