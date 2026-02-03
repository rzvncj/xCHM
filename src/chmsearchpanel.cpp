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

#include <algorithm>
#include <chmhtmlnotebook.h>
#include <chmhtmlwindow.h>
#include <chminputstream.h>
#include <chmlistctrl.h>
#include <chmsearchpanel.h>
#include <hhcparser.h>
#include <vector>
#include <wx/config.h>
#include <wx/sizer.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>
#include <wx/wx.h>

CHMSearchPanel::CHMSearchPanel(wxWindow* parent, wxTreeCtrl* topics, CHMHtmlNotebook* nbhtml)
    : wxPanel(parent), _tcl(topics), _nbhtml(nbhtml)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    SetAutoLayout(true);
    SetSizer(sizer);

    _text = new wxTextCtrl(this, ID_SearchText, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    _partial = new wxCheckBox(this, wxID_ANY, _("Get partial matches"));
    _titles  = new wxCheckBox(this, wxID_ANY, _("Search titles only"));
    _search  = new wxButton(this, ID_SearchButton, _("Search"));

#if wxUSE_TOOLTIPS
    _partial->SetToolTip(_("Allow partial matches."));
    _titles->SetToolTip(_("Only search in the contents' titles."));
    _search->SetToolTip(_("Search contents for occurrences of the specified text."));
#endif
    _results = new CHMListCtrl(this, nbhtml, ID_Results);

    sizer->Add(_text, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 2);
    sizer->Add(_partial, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    sizer->Add(_titles, 0, wxLEFT | wxRIGHT, 10);
    sizer->Add(_search, 0, wxALL, 10);
    sizer->Add(_results, 1, wxALL | wxEXPAND, 2);

    GetConfig();
}

CHMSearchPanel::~CHMSearchPanel()
{
    SetConfig();
}

void CHMSearchPanel::OnSearch(wxCommandEvent&)
{
    wxBusyCursor bcr;

    _results->Reset();

    auto     sr = _text->GetLineText(0);
    wxString word;

    if (sr.IsEmpty())
        return;

    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    sr.MakeLower();

    auto srLen = sr.length();

    for (size_t i = 0; i < srLen; ++i)
        switch (wxChar(sr[i])) {
        case wxT('+'):
        case wxT('-'):
        case wxT('#'):
        case wxT('@'):
        case wxT('^'):
        case wxT('&'):
        case wxT('%'):
            sr[i] = wxT(' ');
            break;
        default:
            break;
        }

    wxStringTokenizer tkz(sr, wxT(" \t\r\n"));

    while (word.IsEmpty())
        if (tkz.HasMoreTokens())
            word = tkz.GetNextToken();

    CHMSearchResults h1;
    chmf->IndexSearch(word, !_partial->IsChecked(), _titles->IsChecked(), h1);

    while (tkz.HasMoreTokens()) {
        auto token = tkz.GetNextToken();
        if (token.IsEmpty())
            continue;

        CHMSearchResults h2, tmp;
        chmf->IndexSearch(token, !_partial->IsChecked(), _titles->IsChecked(), h2);

        if (!h2.empty()) {
            for (auto&& item : h2)
                if (h1.find(item.first) != h1.end())
                    tmp[item.first] = item.second;
            h1 = tmp;
        } else {
            h1.clear();
            break;
        }
    }

    if (_titles->IsChecked() && h1.empty()) {
        PopulateList(_tcl->GetRootItem(), sr, !_partial->IsChecked());
        _results->UpdateItemCount();
        _results->UpdateUI();
        return;
    }

    for (auto&& item : h1) {
        auto url = item.first.StartsWith(wxT("/")) ? item.first : (wxT("/") + item.first);
        _results->AddPairItem(item.second, url);
    }

    _results->UpdateItemCount();
    _results->UpdateUI();
}

void CHMSearchPanel::PopulateList(wxTreeItemId root, const wxString& text, bool wholeWords)
{
    static auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    auto data = dynamic_cast<URLTreeItem*>(_tcl->GetItemData(root));

    if (data && (!data->_url.IsEmpty())) {
        auto title = _tcl->GetItemText(root);
        if (TitleSearch(title, text, wholeWords))
            _results->AddPairItem(title, data->_url);
    }

    wxTreeItemIdValue cookie;

    for (auto child = _tcl->GetFirstChild(root, cookie); child; child = _tcl->GetNextChild(root, cookie)) {
        PopulateList(child, text, wholeWords);
    }
}

bool CHMSearchPanel::TitleSearch(const wxString& title, const wxString& text, bool wholeWords)
{
    auto ncTitle = title;
    auto ncText  = text;

    ncTitle.MakeLower();
    ncText.MakeLower();

    wxStringTokenizer     textTokenizer(ncText), titleTokenizer(ncTitle);
    std::vector<wxString> titleTokens;

    if (wholeWords)
        while (titleTokenizer.HasMoreTokens())
            titleTokens.push_back(titleTokenizer.GetNextToken());

    auto processedTokens = false;

    while (textTokenizer.HasMoreTokens()) {
        auto textToken = textTokenizer.GetNextToken();

        if (textToken.IsEmpty())
            continue;

        processedTokens = true;

        if (!wholeWords) {
            if (ncTitle.find(textToken) == wxString::npos)
                return false;

        } else if (std::find(titleTokens.begin(), titleTokens.end(), textToken) == titleTokens.end())
            return false;
    }

    return processedTokens;
}

void CHMSearchPanel::OnSearchSel(wxListEvent& event)
{
    event.Skip();

    _results->LoadSelected(event.GetIndex());
}

void CHMSearchPanel::OnItemActivated(wxListEvent& event)
{
    event.Skip();

    _results->LoadSelected(event.GetIndex());
    _nbhtml->GetCurrentPage()->SetFocus();
}

void CHMSearchPanel::Reset()
{
    _text->Clear();
    _results->Reset();
}

void CHMSearchPanel::SetNewFont(const wxFont& font)
{
    _results->SetFont(font);
}

void CHMSearchPanel::SetConfig()
{
    wxConfig config(wxT("xchm"));

    config.Write(wxT("/Search/partialWords"), static_cast<long>(_partial->GetValue()));
    config.Write(wxT("/Search/titlesOnly"), static_cast<long>(_titles->GetValue()));
}

void CHMSearchPanel::GetConfig()
{
    long     partial, titles;
    wxConfig config(wxT("xchm"));

    if (config.Read(wxT("/Search/partialWords"), &partial)) {
        config.Read(wxT("/Search/titlesOnly"), &titles);
        _partial->SetValue(partial);
        _titles->SetValue(titles);
    }
}

BEGIN_EVENT_TABLE(CHMSearchPanel, wxPanel)
EVT_LIST_ITEM_SELECTED(ID_Results, CHMSearchPanel::OnSearchSel)
EVT_LIST_ITEM_ACTIVATED(ID_Results, CHMSearchPanel::OnItemActivated)
EVT_BUTTON(ID_SearchButton, CHMSearchPanel::OnSearch)
EVT_TEXT_ENTER(ID_SearchText, CHMSearchPanel::OnSearch)
END_EVENT_TABLE()
