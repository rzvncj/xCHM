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


#ifndef __CHMSEARCHPANEL_HPP_
#define __CHMSEARCHPANEL_HPP_


#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/html/htmlwin.h>
#include <wx/font.h>


/*!
  \class wxPanel
  \brief generic wxWindows panel widget class.
*/


//! IDs for various widget events.
enum {
	ID_SearchText = 1024,
	ID_SearchButton,
	ID_Results,
};


//! Custom built search panel.
class CHMSearchPanel : public wxPanel {

public:
	/*!
	  \brief Initialized the search panel.
	  \param parent Parent widget.
	  \param topics A wxTreeCtrl* that will be iterated over
	  recursively in order to figure out which files from the
	  archive to be searched.
	  \param html The widget that can load a html page that
	  has been found as a result of searching.
	 */
	CHMSearchPanel(wxWindow *parent, wxTreeCtrl *topics,
		       wxHtmlWindow* html);

	//! Calls SetConfig().
	~CHMSearchPanel();

	//! Resets the panel (clears the result list and the textbox.)
	void Reset();

	//! Sets the font.
	void SetNewFont(const wxFont& font);

	//! Resets font.
	void ResetFont();

protected:
	/*! 
	  This gets called when the user clicks the Search button or
	  presses Enter in the textbox.
	*/
	void OnSearch(wxCommandEvent& event);

	//! This gets called when the user clicks on a result.
	void OnSearchSel(wxCommandEvent& event);

private:
	//! Helper. Searches through the tree recursively.
	void PopulateList(wxTreeItemId root, wxString& text, bool wholeWords);

	//! Helper. Grep searches page titles for the given text.
	bool TitleSearch(const wxString& title, wxString& text,
			 bool caseSensitive, bool wholeWords);

	//! Reads the search configuration from .xchm (case sensitive, etc.).
	void GetConfig();

	//! Writes the search configuration to .xchm.
	void SetConfig();

private:
	wxTreeCtrl* _tcl;
	wxTextCtrl* _text;
	wxCheckBox* _partial;
	wxCheckBox* _titles;
	wxButton* _search;
	wxListBox* _results;
	wxHtmlWindow* _html;
	wxFont _font;

private:
	DECLARE_EVENT_TABLE()
};


#endif // __CHMSEARCHPANEL_HPP_

