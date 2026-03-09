/*
  Copyright (C) 2003 - 2026  Razvan Cojocaru <razvanc@mailbox.org>
  ListDirty() patch contributed by Iulian Dragos
  <dragosiulian@users.sourceforge.net>

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
#include <chminputstream.h>
#include <chmlistctrl.h>
#include <wx/settings.h>

namespace {

//! Comparator for sorted insertion: case-insensitive by title.
bool ItemLessThan(const std::unique_ptr<CHMListPairItem>& a, const std::unique_ptr<CHMListPairItem>& b)
{
    return a->_title.CmpNoCase(b->_title) < 0;
}

} // namespace

// CHMListCtrl implementation

CHMListCtrl::CHMListCtrl(wxWindow* parent, CHMHtmlNotebook* nbhtml, wxWindowID id)
    : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                 wxLC_VIRTUAL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxSUNKEN_BORDER),
      _nbhtml(nbhtml)
{
    constexpr size_t INDEX_HINT_SIZE {2048};

    InsertColumn(0, wxEmptyString);
    SetItemCount(0);

    _items.reserve(INDEX_HINT_SIZE);

    Bind(wxEVT_SIZE, &CHMListCtrl::OnSize, this);
}

void CHMListCtrl::Reset()
{
    _items.clear();
    DeleteAllItems();
    UpdateUI();
}

void CHMListCtrl::UpdateItemCount()
{
    SetItemCount(_items.size());
}

void CHMListCtrl::AddPairItem(const wxString& title, const wxString& url)
{
    auto item = std::make_unique<CHMListPairItem>(title, url);
    auto pos  = std::lower_bound(_items.begin(), _items.end(), item, ItemLessThan);
    _items.insert(pos, std::move(item));
}

void CHMListCtrl::LoadSelected()
{
    auto item = -1L;
    item      = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (item == -1L || item >= static_cast<long>(_items.size()))
        return;

    auto chmf = CHMInputStream::GetCache();

    if (chmf) {
        auto fname = _items[item]->_url;

        if (!fname.StartsWith(wxT("file:")))
            fname = wxT("file:") + chmf->ArchiveName() + wxT("#xchm:/") + _items[item]->_url;

        _nbhtml->LoadPageInCurrentView(fname);
    }
}

void CHMListCtrl::UpdateUI()
{
    SetColumnWidth(0, GetClientSize().GetWidth());
}

void CHMListCtrl::FindBestMatch(const wxString& title)
{
    for (size_t i = 0; i < _items.size(); ++i) {
        if (!_items[i]->_title.Left(title.length()).CmpNoCase(title)) {
            EnsureVisible(i);
            SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            break;
        }
    }

    Refresh();
    wxWindow::Update();
}

void CHMListCtrl::OnSize(wxSizeEvent& event)
{
    UpdateUI();
    event.Skip();
}

wxString CHMListCtrl::OnGetItemText(long item, long column) const
{
    // Is this even possible? item == -1 or item > size - 1?
    if (column != 0 || item == -1L || item >= static_cast<long>(_items.size()))
        return wxT("");

    return _items[item]->_title;
}
