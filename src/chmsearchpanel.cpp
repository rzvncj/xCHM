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

#include <chmsearchpanel.h>
#include <wx/sizer.h>


// !! Watch it, the tree might be empty. Also, remember to check
// for nodes with NULL userdata.

CHMSearchPanel::CHMSearchPanel(wxWindow *parent, wxTreeCtrl *topics)
	: wxPanel(parent), _tcl(topics), _text(NULL), _case(NULL),
	  _whole(NULL), _search(NULL), _results(NULL)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetAutoLayout(TRUE);
        SetSizer(sizer);

	_text = new wxTextCtrl(this, ID_SearchText, wxEmptyString, 
			       wxDefaultPosition, wxDefaultSize, 
			       wxTE_PROCESS_ENTER);

	_case = new wxCheckBox(this, -1, wxT("Case sensitive"));
	_whole = new wxCheckBox(this, -1, wxT("Whole words only"));	
	_search = new wxButton(this, ID_SearchButton, wxT("Search"));

	_search->SetToolTip(wxT("Search contents for all occurences of"
				" the specified text."));

	_results = new wxListBox(this, ID_Results, 
				 wxDefaultPosition, wxDefaultSize, 
				 0, NULL, wxLB_SINGLE);

        sizer->Add(_text, 0, wxEXPAND | wxALL, 10);
        sizer->Add(_case, 0, wxLEFT | wxRIGHT, 10);
        sizer->Add(_whole, 0, wxLEFT | wxRIGHT, 10);
	sizer->Add(_search, 0, wxALL, 10);
        sizer->Add(_results, 1, wxALL | wxEXPAND, 2);
}


void CHMSearchPanel::Reset()
{
	_text->Clear();
	_case->SetValue(FALSE);
	_whole->SetValue(FALSE);
	_results->Clear();
}

