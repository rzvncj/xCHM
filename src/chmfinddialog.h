/*

  Copyright (C) 2003 - 2013  Razvan Cojocaru <rzvncj@gmail.com>
 
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


#ifndef __CHMFINDDIALOG_H
#define __CHMFINDDIALOG_H


#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/html/htmlcell.h>


// Forward declarations.
class wxCheckBox;
class CHMHtmlWindow;


//! Event IDs.
enum {
	ID_TextFind = 1408,
	ID_FindNext,
};


//! Dialog for finding a word in the currently displayed page.
class CHMFindDialog : public wxDialog {
public:
	//! Initializes the dialog.
	CHMFindDialog(wxWindow *parent, CHMHtmlWindow *toSearch);

	//! Sets the focus to the textbox.
	void SetFocusToTextBox() { _text->SetFocusFromKbd(); }

	//! Resets the word to be found, so 'Find next' will start over.
	void Reset() { _cell = NULL; }

protected:
	//! Called when the user clicks the 'Find next' button.
	void OnFind(wxCommandEvent& event);

private:
	CHMHtmlWindow* _html;
	wxTextCtrl* _text;	
	wxCheckBox* _whole;
	wxCheckBox* _case;
	wxString _currWord;
	wxHtmlCell *_cell;

private:
	DECLARE_EVENT_TABLE();
};


#endif // __CHMFINDDIALOG_H


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

