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


#include <chmfinddialog.h>
#include <chmhtmlwindow.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/tokenzr.h>
#include <wx/wx.h>


CHMFindDialog::CHMFindDialog(wxWindow *parent, CHMHtmlWindow *toSearch)
	: wxDialog(parent, -1, wxString(_("Find in page.."))),
	  _html(toSearch), _cell(NULL)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	_text = new wxTextCtrl(this, ID_TextFind, wxEmptyString, 
			       wxDefaultPosition, wxSize(200, -1), 
			       wxTE_PROCESS_ENTER);

	_whole = new wxCheckBox(this, -1, _("Whole words only"));
	_case = new wxCheckBox(this, -1, _("Case sensitive"));

	sizer->Add(_text, 0, wxLEFT | wxTOP | wxBOTTOM, 5);
	sizer->Add(_whole, 0, wxLEFT, 5);
	sizer->Add(_case, 0, wxLEFT | wxBOTTOM, 5);

	wxSizer *szButtons = new wxBoxSizer(wxVERTICAL);
	wxButton *find = new wxButton(this, ID_FindNext, _("Find next"));

	szButtons->Add(find, 1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
	szButtons->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 
		       1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);

	wxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
	topsizer->Add(sizer);
	topsizer->Add(szButtons);

	SetAutoLayout(TRUE);
	SetSizer(topsizer);
	topsizer->Fit(this);
	Centre(wxBOTH);

	SetFocusToTextBox();
}


void CHMFindDialog::OnFind(wxCommandEvent& WXUNUSED(event))
{	
	_html->ClearSelection();

	wxString sr = _text->GetLineText(0);
	if (sr.IsEmpty())
		return;

	wxStringTokenizer tkz(sr, wxT(" \t\r\n"));
	wxString word;

	while(word.IsEmpty())
		if(tkz.HasMoreTokens())
			word = tkz.GetNextToken();

	if(!_case->IsChecked())
		word.MakeLower();

	if(!_cell || word.Cmp(_currWord.c_str())) {
		_cell = _html->FindFirst(_html->GetInternalRepresentation(),
					 word, _whole->IsChecked(),
					 _case->IsChecked());
		_currWord = word;

	} else {

		if(_cell && _cell->GetNext()) 
			_cell = _cell->GetNext();
	        else {
			while(_cell && !_cell->GetNext())
				_cell = _cell->GetParent();

			if(_cell)
				_cell = _cell->GetNext();
		}

		if(!_cell)
			return;

		_cell = _html->FindNext(_cell, word, _whole->IsChecked(), 
				       _case->IsChecked());

		// Wrap around.
		if(!_cell) {
			_cell = _html->FindFirst(
				_html->GetInternalRepresentation(),
				word, _whole->IsChecked(),
				_case->IsChecked());
		}
	}
}



BEGIN_EVENT_TABLE(CHMFindDialog, wxDialog)
    EVT_TEXT_ENTER(ID_TextFind, CHMFindDialog::OnFind)
    EVT_BUTTON(ID_FindNext, CHMFindDialog::OnFind)
END_EVENT_TABLE()


