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


#ifdef _ENABLE_COPY_AND_FIND
#ifndef __CHMFINDDIALOG_H
#define __CHMFINDDIALOG_H


#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/html/htmlcell.h>


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
#endif // _ENABLE_COPY_AND_FIND
