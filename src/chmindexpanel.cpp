/*
  Copyright (C) 2003 - 2026  Razvan Cojocaru <razvanc@mailbox.org>

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

#include <chmhtmlnotebook.h>
#include <chmhtmlwindow.h>
#include <chmindexpanel.h>
#include <chmlistctrl.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

CHMIndexPanel::CHMIndexPanel(wxWindow* parent, CHMHtmlNotebook* nbhtml) : wxPanel(parent), _nbhtml(nbhtml)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    SetAutoLayout(true);
    SetSizer(sizer);

    _text = new wxTextCtrl(this, ID_SearchIndex, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    _lc   = new CHMListCtrl(this, nbhtml, ID_IndexClicked);

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

void CHMIndexPanel::OnIndexSel(wxListEvent& event)
{
    event.Skip();

    if (_navigate)
        _lc->LoadSelected(event.GetIndex());
}

void CHMIndexPanel::OnItemActivated(wxListEvent& event)
{
    event.Skip();

    if (_navigate)
        _lc->LoadSelected(event.GetIndex());
    _nbhtml->GetCurrentPage()->SetFocus();
}

void CHMIndexPanel::OnText(wxCommandEvent&)
{
    _navigate = false;
    _lc->FindBestMatch(_text->GetLineText(0));
    _navigate = true;
}

void CHMIndexPanel::OnTextEnter(wxCommandEvent&)
{
    _navigate = false;
    _lc->FindBestMatch(_text->GetLineText(0));
    _lc->LoadSelected(_lc->GetFocusedItem());
    _nbhtml->GetCurrentPage()->SetFocusFromKbd();
    _navigate = true;
}

BEGIN_EVENT_TABLE(CHMIndexPanel, wxPanel)
EVT_TEXT(ID_SearchIndex, CHMIndexPanel::OnText)
EVT_TEXT_ENTER(ID_SearchIndex, CHMIndexPanel::OnTextEnter)
EVT_LIST_ITEM_SELECTED(ID_IndexClicked, CHMIndexPanel::OnIndexSel)
EVT_LIST_ITEM_ACTIVATED(ID_IndexClicked, CHMIndexPanel::OnItemActivated)
END_EVENT_TABLE()
