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


#ifndef __CHMHTMLWINDOW_H_
#define __CHMHTMLWINDOW_H_


#include <wx/html/htmlwin.h>
#include <wx/treectrl.h>


/*!
  \class wxHtmlWindow
  \brief wxWindows HTML widget class.
*/


/*! 
  \brief Custom HTML widget class. Needed for sychnronization between
  the topics tree control and the currently displayed page.
*/
class CHMHtmlWindow : public wxHtmlWindow {

public:
	/*!
	  \brief Initializes the widget.
	  \param parent The parent widget.
	  \param tc Pointer to the tree control we want to synchronize
	  with.
	 */
	CHMHtmlWindow(wxWindow *parent, wxTreeCtrl *tc);

	//! Override. Looks up the wanted page in the tree and selects it.
	bool LoadPage(const wxString& location);

	/*!
	  \brief Dictates the behaviour of LoadPage(). If SetSync()
	  has been called with a true parameter, the tree control will
	  be updated by LoadPage(). Otherwise, it will not be updated.
	  \param value Synchronize the tree widget on load?
	 */
	void SetSync(bool value) { _syncTree = value; }
	
	/*!
	  Returns true if the tree control's EVT_TREE_SEL_CHANGED
	  event happened as a result of the CHMHtmlWindow calling
	  SelectItem() on it.
	*/
	bool IsCaller() { return _found; }

private:
	//! Helper. Recursively looks for the opened page in the tree.
	void Sync(wxTreeItemId root, const wxString& page);

	//! Helper. Returns the prefix of the currently loaded page.
	wxString GetPrefix();
	
private:
	wxTreeCtrl* _tcl;
	bool _syncTree;
	bool _found;
};


#endif // __CHMHTMLWINDOW_H_

