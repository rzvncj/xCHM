/*
  Copyright (C) 2003 - 2026  Razvan Cojocaru <razvanc@mailbox.org>

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

#ifndef __CHMFRAME_H_
#define __CHMFRAME_H_

#include <array>
#include <memory>
#include <wx/combobox.h>
#include <wx/docview.h>
#include <wx/font.h>
#include <wx/html/htmprint.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/thread.h>
#include <wx/treectrl.h>
#include <wx/wx.h>

// Forward declarations.
class CHMHtmlWindow;
class CHMSearchPanel;
class CHMIndexPanel;
class wxFileType;
class CHMHtmlNotebook;

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
// this should never happen
#define VERSION "unknown"
#endif

//! Default font size for the wxHtmlWindow.
constexpr int CHM_DEFAULT_FONT_SIZE {12};

//! Default sash position.
constexpr int CONTENTS_MARGIN {170};

//! IDs for various widget events.
enum {
    ID_Quit = 1,
    ID_About,
    ID_Open,
    ID_Fonts,
    ID_Print,
    ID_Home,
    ID_Forward,
    ID_Back,
    ID_Contents,
    ID_Bookmarks,
    ID_Add,
    ID_Remove,
    ID_FindInPage,
    ID_CopySelection,
    ID_Recent,
    ID_Notebook,
    ID_RegisterExtension,
    ID_CloseTab,
    ID_NewTab,
    ID_FullScreen,
    ID_ToggleToolbar,
    ID_TreeCtrl = 1000,
};

/*!
  \class wxFrame
  \brief wxWidgets frame widget class.
*/

using FontSizesArray = std::array<int, 7>;

//! The frame, owner of the wxHtmlWindow, contents tree control and all the other nifty stuff.
class CHMFrame : public wxFrame {

public:
    /*!
      \brief Brings the frame into existence.
      \param title The text that shows up on the titlebar.
      \param booksDir Where to go when you click Open.. on the toolbar. This is used to remember the last directory
      where a book was sucessfully opened. For the current working directory just pass the empty string.
      \param pos The upper left corner of the frame.
      \param size The size of the frame.
      \param normalFont Name of the font face to use for normal text.
      \param fixedFont Name of the font face to use for fixed text.
      \param fontSize The font size.
      \param sashPosition Distance from the left of the frame to
      \param fullAppPath The absolute path to the executable of the process the end of the contents / search panel.
      \param loadTopics If set to false, don't try to load the topics tree.
      \param loadIndex If set to false, don't try to load the index list.
    */
    CHMFrame(const wxString& title, const wxString& booksDir, const wxPoint& pos, const wxSize& size,
             const wxString& normalFont = wxEmptyString, const wxString& fixedFont = wxEmptyString,
             int fontSize = CHM_DEFAULT_FONT_SIZE, int sashPosition = CONTENTS_MARGIN,
             const wxString& fullAppPath = wxEmptyString, bool loadTopics = true, bool loadIndex = true);

    //! Cleans up.
    ~CHMFrame();

    /*!
      \brief Attempts to load a .chm file and display it's home page.
      \param archive The .chm file name on disk.
      \return true if the operation was successful.
     */
    bool LoadCHM(const wxString& archive);

    /*!
      \brief Attempts to load a context-ID from within the current chm file
      \param contextID the context-ID to load.
      \return true if the operation was successful.
     */
    bool LoadContextID(int contextID);

    //! Fills the index and the contents tree.
    void UpdateCHMInfo();

    //! Add html view
    void AddHtmlView(const wxString& path, const wxString& link);

    //! Toggles fullscreen mode
    void ToggleFullScreen(bool onlyIfFullScreenOn = false);

    FontSizesArray ComputeFontSizes(int size) const;

protected:
    //! Called when the user closes the window.
    void OnQuit(wxCommandEvent& event);

    //! Called when the user clicks on About.
    void OnAbout(wxCommandEvent& event);

    //! Called when the user wants to open a file.
    void OnOpen(wxCommandEvent& event);

    //! Called when the user wants to change the fonts.
    void OnChangeFonts(wxCommandEvent& event);

    //! Called when the user wants to see the default page.
    void OnHome(wxCommandEvent& event);

    //! Called when the user wants to go forward in the history.
    void OnHistoryForward(wxCommandEvent& event);

    //! Called when the user wants to go back in the history.
    void OnHistoryBack(wxCommandEvent& event);

    //! Called when the user wants to either disable or enable the contents tree panel on the left.
    void OnShowContents(wxCommandEvent& event);

    //! Called when the user wants to print the displayed page.
    void OnPrint(wxCommandEvent& event);

    //! Called when the user selects a file from the file history.
    void OnHistFile(wxCommandEvent& event);

    //! Called when the user types Ctrl-F.
    void OnFind(wxCommandEvent& event);

    //! Called when the user types Ctrl-F.
    void OnCopySelection(wxCommandEvent& event);

    //! Called when fullscreen mode is being toggled
    void OnToggleFullScreen(wxCommandEvent& event);

    //! Called when the toolbar is being toggled
    void OnToggleToolbar(wxCommandEvent& event);

    //! Called when the user clicks on the Add button.
    void OnAddBookmark(wxCommandEvent& event);

#if defined(__WXMSW__) || defined(__WXMAC__)
    //! Called when the user selects Register extension
    void OnRegisterExtension(wxCommandEvent& event);
#endif // __WXMSW__

    //! Called when the user clicks on the Remove button.
    void OnRemoveBookmark(wxCommandEvent& event);

    //! Called when the user chooses a bookmark from the wxChoice control.
    void OnBookmarkSel(wxCommandEvent& event);

    //! Called when an item in the contents tree is selected.
    void OnSelectionChanged(wxTreeEvent& event);

    //! Called when an item in the contents tree is activated.
    void OnItemActivated(wxTreeEvent& event);

    //! Cleanup code. This saves the window position and last open dir.
    void OnCloseWindow(wxCloseEvent& event);

    //! Called when the user presses a key
    void OnChar(wxKeyEvent& event);

    //! Called when the user types Ctrl-F4.
    void OnCloseTab(wxCommandEvent& event);

    //! Called when the user types Ctrl-N.
    void OnNewTab(wxCommandEvent& event);

private:
    //! Helper. Creates the menu.
    wxMenuBar* CreateMenu();

    //! Helper. Initializes the frame toolbar.
    bool InitToolBar(wxToolBar* toolbar);

    //! Helper. Creates the contents panel.
    wxPanel* CreateContentsPanel();

    //! Helper. Loads the bookmarks for the currently opened CHM file.
    void LoadBookmarks();

    //! Helper. Saves the bookmarks for the currently opened CHM file.
    void SaveBookmarks();

    //! Helper. Saves exit information (size, history, etc.)
    void SaveExitInfo();

    void LoadSelected(wxTreeItemId id);

private:
    CHMHtmlNotebook*                    _nbhtml;
    wxTreeCtrl*                         _tcl {nullptr};
    wxSplitterWindow*                   _sw {nullptr};
    wxMenu*                             _menuFile {nullptr};
    wxToolBar*                          _tb {nullptr};
    std::unique_ptr<wxHtmlEasyPrinting> _ep;
    wxNotebook*                         _nb {nullptr};
    wxComboBox*                         _cb {nullptr};
    CHMSearchPanel*                     _csp {nullptr};
    CHMIndexPanel*                      _cip {nullptr};

    wxString      _openPath;
    wxString      _normalFont;
    wxString      _fixedFont;
    bool          _bookmarkSel {true};
    bool          _bookmarksDeleted {false};
    int           _sashPos;
    wxFont        _font;
    wxFileHistory _fh;
    wxString      _fullAppPath;
    bool          _loadTopics;
    bool          _loadIndex;
    bool          _fullScreen {false};

private:
    DECLARE_EVENT_TABLE()
};

#endif // __CHMFRAME_H_
