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


#include <chmindexpanel.h>
#include <chmhtmlwindow.h>
#include <chmlistctrl.h>
#include <wx/sizer.h>


CHMIndexPanel::CHMIndexPanel(wxWindow *parent, CHMHtmlWindow *html)
	: wxPanel(parent), _html(html), _lc(NULL), _navigate(true)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetAutoLayout(TRUE);
        SetSizer(sizer);

	_text = new wxTextCtrl(this, ID_SearchIndex, wxEmptyString, 
			       wxDefaultPosition, wxDefaultSize, 
			       wxTE_PROCESS_ENTER);

	_lc = new CHMListCtrl(this, html, ID_IndexClicked);

        sizer->Add(_text, 0, wxEXPAND | wxALL, 2);
        sizer->Add(_lc, 1, wxALL | wxEXPAND, 2);
}


void CHMIndexPanel::Reset()
{
	_text->Clear();
	_lc->Reset();
}


void CHMIndexPanel::SetNewFont(const wxFont& font)
{
	_lc->SetFont(font);
}


void CHMIndexPanel::OnIndexSel(wxCommandEvent& WXUNUSED(event))
{
	if(_navigate)
		_lc->LoadSelected();
}


void CHMIndexPanel::OnText(wxCommandEvent& WXUNUSED(event))
{
	wxListItem info;
	info.m_col = 0;

	long sz = _lc->GetItemCount();
	wxString typedText = _text->GetLineText(0);
	int tl = typedText.length();

	for(long i = 0; i < sz; ++i) {

		info.m_itemId = i;
		_lc->GetItem(info);

		if(!info.m_text.Left(tl).CmpNoCase(typedText)) {

			_navigate = false;
			_lc->SetItemState(i, wxLIST_STATE_SELECTED,
					  wxLIST_STATE_SELECTED);
			_navigate = true;
			_lc->EnsureVisible(i);
			break;
		}
	}

	_lc->Refresh();
}


BEGIN_EVENT_TABLE(CHMIndexPanel, wxPanel)
	EVT_TEXT(ID_SearchIndex, CHMIndexPanel::OnText)
	EVT_TEXT_ENTER(ID_SearchIndex, CHMIndexPanel::OnIndexSel)
	EVT_LIST_ITEM_SELECTED(ID_IndexClicked, CHMIndexPanel::OnIndexSel)
END_EVENT_TABLE()


