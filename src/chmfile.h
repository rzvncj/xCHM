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

#ifndef __CHMFILE_H_
#define __CHMFILE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#ifdef ENABLE_BUILTIN_CHMLIB
#include "xchm_chm_lib.h"
#else
#include <chm_lib.h>
#endif
#include <string>
#include <vector>
#include <wx/filefn.h>
#include <wx/font.h>
#include <wx/hashmap.h>
#include <wx/string.h>

// Forward declarations.
class wxTreeCtrl;
class CHMListCtrl;
class wxCSConv;

using UCharVector = std::vector<unsigned char>;

//! Declares a class called CHMSearchResults - <string, string> hashmap.
WX_DECLARE_STRING_HASH_MAP(wxString, CHMSearchResults);
//! Declares a class called CHMIDMap - <int, string> hashmap.
WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, CHMIDMap);

//! C++ wrapper around CHMLIB. Concrete class.
class CHMFile {
    //! Helper. To avoid a large list of parameters in 'ProcessWLC', and slightly improve readability
    struct IndexSearchUnitsInfo {
        chmFile*    fileMain;
        chmUnitInfo uiMain {};

        chmFile*    fileTopics;
        chmUnitInfo uiTopics {};

        chmFile*    fileStrings;
        chmUnitInfo uiStrings {};

        chmFile*    fileUrltbl;
        chmUnitInfo uiUrltbl {};

        chmFile*    fileUrlstr;
        chmUnitInfo uiUrlstr {};
    };

public:
    //! Default constructor.
    CHMFile() = default;

    /*!
      \brief This constructor attempts to open the .chm file passed as it's argument. If it fails, IsOk() will
      return false.
      \param archiveName The .chm filename on disk.
     */
    explicit CHMFile(const wxString& archiveName);

    //! Destructor. If a file has been succesfully opened, it closes it.
    ~CHMFile();

    /*!
      \brief Gets the name of the default page in the archive.
      \return The home page name, with a '/' added in front and relative to the root of the archive filesystem. If
      no .chm has been opened, the returned value is "/".
     */
    wxString HomePage() const { return _home; }

    /*!
      \brief Gets the filename of the currently opened .chm file.
      \return The filename of the currently opened archive, relative to the root of the filesystem, or the empty string
      if no archive has been opened.
     */
    wxString ArchiveName() const { return _filename; }

    /*!
      \brief Gets the name of the opened .chm.
      \return The name of the opened document, or an empty string if no .chm has been loaded.
    */
    wxString Title() const { return _title; }

    /*!
      \brief Checks if the last attempt to load a .chm file was succesful.
      \return true, if the last attempt to load a .chm file was succesful, false otherwise.
     */
    bool IsOk() const { return _chmFile; }

    //! Checks if a 'ms-its' link has been clicked and the chm changed.
    bool HasChanged();

    /*!
      \brief Determines the encoding that the CHM creator would like me to use. To be used in combination with
      DefaultFont().
      \return Desired encoding.
     */
    wxFontEncoding DesiredEncoding() const { return _enc; }

    /*!
      \brief Determines the font to use for special charsets.
      \return Detected font recommendation.
     */
    wxString DefaultFont() const { return _font; }

    /*!
      \brief Attempts to load a .chm file from disk.
      \param archiveName The .chm filename on disk.
      \return true on success, false on failure.
     */
    bool LoadCHM(const wxString& archiveName);

    //! Closes the currently opened .chm, or does nothing if none opened.
    void CloseCHM();

    /*!
      \brief Attempts to fill a wxTreeCtrl by parsing the topics file.
      \param toBuild Pointer to the tree to be filled. If the topics file is not available, the tree is unmodified.
      The tree must be empty before passing it to this function.
      \return true if it's possible to build the tree, false otherwise.
     */
    bool GetTopicsTree(wxTreeCtrl& toBuild);

    /*!
      \brief Attempts to fill a CHMListCtrl by parsing the index file.
      \param toBuild Pointer to the list control to be filled. If the index file is not available, the list control
      is unmodified. The list must be empty before passing it to this function.
      \return true if it's possible to build the tree, false otherwise.
     */
    bool GetIndex(CHMListCtrl& toBuild);

    /*!
      \brief Attempts to build an index of context-ID/page pairs from the file.
      \return true if it's possible to buld the tree, false otherwise.
     */
    bool LoadContextIDs();

    /*!
      \brief Checks whether or not the context ID is valid for the loaded file.
      \param contextID The context-ID to check.
      \return true if the context ID is valid. false otherwise.
     */
    bool IsValidCID(int contextID);

    /*!
     \brief Looks up the page referred to by the context-ID
     \param contextID The context-ID to look up
     \return the page referred to by the context-ID, or file root"/"  if ID is invalid.
     */
    wxString GetPageByCID(int contextID);

    /*!
      \brief Fast search using the $FIftiMain file in the .chm.
      \param text The text we're looking for.
      \param wholeWords Are we looking for whole words only?
      \param titlesOnly Are we looking for titles only?
      \param results A string-string hashmap that will hold the results in case of successful search. The keys are
      the URLs and the values are the page titles.
      \return true if the search succeeded, false otherwise.
     */
    bool IndexSearch(const wxString& text, bool wholeWords, bool titlesOnly, CHMSearchResults& results);

    /*!
      \brief Looks up fileName in the archive.
      \param fileName The file name to look up in the archive, qualified with '/' standing for the root of the archive
      filesystem.
      \param ui A pointer to CHMLIB specific data about the file. The parameter gets filled with useful data if the
      lookup was succesful.
      \return true if the file exists in the archive, false otherwise.
     */
    bool ResolveObject(const wxString& fileName, chmUnitInfo* ui);

    /*!
      \brief Retrieves an uncompressed chunk of a file in the .chm.
      \param ui Pointer to a CHMLIB specific data structure obtained from a succesful call to ResolveObject().
      \param buffer The buffer to place the chunk into.
      \param fileOffset Where does the chunk we want begin in the file?
      \param bufferSize The size of the buffer.
      \return 0 on error, length of chunk retrieved otherwise.
     */
    size_t RetrieveObject(chmUnitInfo* ui, unsigned char* buffer, off_t fileOffset, size_t bufferSize);

public:
    //! No copy construction allowed.
    CHMFile(const CHMFile&) = delete;

    //! No assignments.
    CHMFile& operator=(const CHMFile&) = delete;

private:
    //! Helper. Translates from Win32 encodings to generic wxWidgets ones.
    wxFontEncoding GetFontEncFromCharSet(int cs);

    //! Helper. Translates from MS-specific LCID.
    wxFontEncoding GetFontEncFromLCID(uint32_t lcid);

    //! Helper. Initializes most of the private data members.
    bool GetArchiveInfo();

    //! Helper. Returns the $FIftiMain offset of leaf node or 0.
    uint32_t GetLeafNodeOffset(const wxString& text, uint32_t initalOffset, uint32_t buffSize, uint16_t treeDepth,
                               chmFile* file, chmUnitInfo* ui);

    //! Helper. Processes the word location code entries while searching.
    bool ProcessWLC(uint64_t wlc_count, uint64_t wlc_size, uint32_t wlc_offset, unsigned char ds, unsigned char dr,
                    unsigned char cs, unsigned char cr, unsigned char ls, unsigned char lr, IndexSearchUnitsInfo& uis,
                    CHMSearchResults& results);

    //! Looks up as much information as possible from #WINDOWS/#STRINGS.
    bool InfoFromWindows();

    //! Looks up as much information as possible from #SYSTEM.
    bool InfoFromSystem();

    //! Load binary TOC (if available)
    bool BinaryTOC(wxTreeCtrl& toBuild);

    //! Try to recursively load the binary topics tree
    void RecurseLoadBTOC(UCharVector& topidx, UCharVector& topics, UCharVector& strings, UCharVector& urltbl,
                         UCharVector& urlstr, uint32_t offset, wxTreeCtrl& toBuild, int level);

    //! Retrieve the data (name/URL) for a single entry (TOC or index)
    bool GetItem(UCharVector& topics, UCharVector& strings, UCharVector& urltbl, UCharVector& urlstr, uint32_t index,
                 wxTreeCtrl* tree, CHMListCtrl* list, const wxString& idxName, int level, bool local);

    //! Get the binary index (if available)
    bool BinaryIndex(CHMListCtrl& toBuild, const wxCSConv& cv);

private:
    chmFile*       _chmFile {nullptr};
    chmFile*       _chmChiFile {nullptr};
    wxString       _filename;
    wxString       _home {wxT("/")};
    wxString       _topicsFile;
    wxString       _indexFile;
    wxString       _title;
    wxString       _font;
    wxFontEncoding _enc;
    CHMIDMap       _cidMap;
};

#endif // __CHMFILE_H_
