/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
  Tabbed browsing support developed by Ced Boudinet <bouced@gmx.fr>
  (this file originally written by Ced Boudinet)
 
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
	CHMHtmlNotebook(wxWindow *parent, wxTreeCtrl *tc, CHMFrame* frame);

	//! Add a notebook tab and display the specified URL
	void AddHtmlView(const wxString& path,
			 const wxString& link);

	//! Displays the URL in the current tab
	bool LoadPageInCurrentView(const wxString& location);

	//! Returns the current page as a CHMHtmlWindow
	CHMHtmlWindow* GetCurrentPage();

	//! Callback for when a child's title changes
	void OnChildrenTitleChanged(const wxString& title);

	//! Close all pages except the first one
	void CloseAllPagesExceptFirst();

	//! Propagate font settings to the children
	void SetChildrenFonts(const wxString& normal_face, 
			      const wxString& fixed_face, 
			      const int *sizes = NULL);

	//! Called when user asks for a tab to close
	void OnCloseTab(wxCommandEvent&);

	//! Called when user ask for a new tab
	void OnNewTab(wxCommandEvent& event);

	//! Creates a new tab view
	CHMHtmlWindow* CreateView();

	//! Overload for tab height control
	virtual bool AddPage(wxWindow* page, const wxString& title);

protected:
	//! Called when user asks for next notebook page
	void OnGoToNextPage(wxCommandEvent&);

	//! Called when user asks for prior notebook page
	void OnGoToPriorPage(wxCommandEvent&);

	//! Callback for the page changed wxWidgets event
	void OnPageChanged(wxAuiNotebookEvent&);

private:
	wxTreeCtrl* _tcl;
	CHMFrame *_frame;
	wxString _fonts_normal_face;
	wxString _fonts_fixed_face;
	int _fonts_sizes[7];
	DECLARE_EVENT_TABLE()
};


#endif // __CHMHTMLNOTEBOOK_H_


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


