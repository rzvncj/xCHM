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


#include <wx/sizer.h>
#include <chmindexpanel.h>


CHMIndexPanel::CHMIndexPanel(wxWindow *parent, CHMHtmlWindow *html)
	: wxPanel(parent), _html(html), _list(NULL)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetAutoLayout(TRUE);
        SetSizer(sizer);

	_text = new wxTextCtrl(this, ID_SearchIndex, wxEmptyString, 
			       wxDefaultPosition, wxDefaultSize, 
			       wxTE_PROCESS_ENTER);

	_list = new wxListBox(this, ID_IndexList, 
			      wxDefaultPosition, wxDefaultSize, 
			      0, NULL, wxLB_SINGLE | wxLB_HSCROLL |
			      wxLB_NEEDED_SB);

        sizer->Add(_text, 0, wxEXPAND | wxALL, 2);
        sizer->Add(_list, 1, wxALL | wxEXPAND, 2);
}


CHMIndexPanel::~CHMIndexPanel()
{}


void CHMIndexPanel::Reset()
{
	_listUrls.Clear();
	_text->Clear();
	_list->Clear();
}


void CHMIndexPanel::SetNewFont(const wxFont& font)
{
	_list->SetFont(font);
}



void CHMIndexPanel::OnIndexSel(wxCommandEvent& WXUNUSED(event))
{
	int sel = _list->GetSelection();

	if(sel < 0)
		return;

	_html->AbsoluteFollows(true);
	_html->LoadPage(_listUrls[sel]);
}


void CHMIndexPanel::OnText(wxCommandEvent& WXUNUSED(event))
{
	int sz = _list->GetCount();
	wxString typedText = _text->GetLineText(0);

	for(int i = 0; i < sz; ++i) {
		wxString str = _list->GetString(i);
		
		if(!str.Left(typedText.length()).CmpNoCase(typedText)) {
			_list->SetSelection(i);
			_list->SetFirstItem(i);
			break;
		}
	}
}


BEGIN_EVENT_TABLE(CHMIndexPanel, wxPanel)
	EVT_TEXT(ID_SearchIndex, CHMIndexPanel::OnText)
	EVT_LISTBOX(ID_IndexList, CHMIndexPanel::OnIndexSel)
	EVT_TEXT_ENTER(ID_SearchIndex, CHMIndexPanel::OnIndexSel)
END_EVENT_TABLE()


