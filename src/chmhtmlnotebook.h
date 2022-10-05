/*
  Copyright (C) 2003 - 2022  Razvan Cojocaru <rzvncj@gmail.com>
  Tabbed browsing support developed by Cedric Boudinet <bouced@gmx.fr>
  (this file originally written by Cedric Boudinet)

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

#ifndef __CHMHTMLNOTEBOOK_H_
#define __CHMHTMLNOTEBOOK_H_

#include <wx/aui/auibook.h>
#include <wx/treectrl.h>

enum {
    ID_NextPage,
    ID_PriorPage,
    ID_ZoomIn,
    ID_ZoomOut,
};

// Forward declarations
class CHMFrame;
class CHMHtmlWindow;

/*!
  \class wxNotebook
  \brief wxWidgets application class.
*/

/*!
  \brief Custom HTML notebook widget class. For tabbed viewing of a CHM file.
*/
class CHMHtmlNotebook : public wxAuiNotebook {

public:
    //! Constructor
    CHMHtmlNotebook(wxWindow* parent, wxTreeCtrl* tc, const wxString& normalFont, const wxString& fixedFont,
                    int fontSize, CHMFrame* frame);

    //! Add a notebook tab and display the specified URL
    void AddHtmlView(const wxString& path, const wxString& link);

    //! Displays the URL in the current tab
    bool LoadPageInCurrentView(const wxString& location);

    //! Returns the current page as a CHMHtmlWindow
    CHMHtmlWindow* GetCurrentPage();

    //! Close all pages except the first one
    void CloseAllPagesExceptFirst();

    //! Propagate font settings to the children
    void SetChildrenFonts(const wxString& normalFace, const wxString& fixedFace, int sizes);

    //! Called when user asks for a tab to close
    void OnCloseTab(wxAuiNotebookEvent&);

    //! Called when user ask for a new tab
    void OnNewTab(wxCommandEvent& event);

    void OnZoomIn(wxCommandEvent& event);

    void OnZoomOut(wxCommandEvent& event);

    //! Creates a new tab view
    CHMHtmlWindow* CreateView();

    //! Add page and check tab height control
    bool AddTab(wxWindow* page, const wxString& title);

    int FontSize() const { return _fontSize; }

protected:
    //! Called when user asks for next notebook page
    void OnGoToNextPage(wxCommandEvent&);

    //! Called when user asks for prior notebook page
    void OnGoToPriorPage(wxCommandEvent&);

private:
    wxTreeCtrl* _tcl;
    CHMFrame*   _frame;
    wxString    _fontsNormalFace;
    wxString    _fontsFixedFace;
    int         _fontSize;
    DECLARE_EVENT_TABLE()
};

#endif // __CHMHTMLNOTEBOOK_H_
