/*
  Copyright (C) 2003 - 2024  Razvan Cojocaru <rzvncj@gmail.com>
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

// Helper

int CompareItemPairs(CHMListPairItem* item1, CHMListPairItem* item2)
{
    return (item1->_title).CmpNoCase(item2->_title);
}

// CHMListCtrl implementation

CHMListCtrl::CHMListCtrl(wxWindow* parent, CHMHtmlNotebook* nbhtml, wxWindowID id)
    : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                 wxLC_VIRTUAL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxSUNKEN_BORDER),
      _items(CompareItemPairs), _nbhtml(nbhtml)
{
    constexpr size_t INDEX_HINT_SIZE {2048};

    InsertColumn(0, wxEmptyString);
    SetItemCount(0);

    _items.Alloc(INDEX_HINT_SIZE);
}

CHMListCtrl::~CHMListCtrl()
{
    // Clean the items up.
    ResetItems();
}

void CHMListCtrl::Reset()
{
    ResetItems();
    DeleteAllItems();
    UpdateUI();
}

void CHMListCtrl::UpdateItemCount()
{
    SetItemCount(_items.GetCount());
}

void CHMListCtrl::AddPairItem(const wxString& title, const wxString& url)
{
    _items.Add(new CHMListPairItem(title, url));
}

void CHMListCtrl::LoadSelected()
{
    auto item = -1L;
    item      = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (item == -1L || item > static_cast<long>(_items.GetCount()) - 1)
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

void CHMListCtrl::ResetItems()
{
    for (auto i = 0UL; i < _items.GetCount(); ++i)
        delete _items[i];

    _items.Empty();
}

void CHMListCtrl::OnSize(wxSizeEvent& event)
{
    UpdateUI();
    event.Skip();
}

wxString CHMListCtrl::OnGetItemText(long item, long column) const
{
    // Is this even possible? item == -1 or item > size - 1?
    if (column != 0 || item == -1L || item > static_cast<long>(_items.GetCount()) - 1)
        return wxT("");

    return _items[item]->_title;
}

BEGIN_EVENT_TABLE(CHMListCtrl, wxListCtrl)
EVT_SIZE(CHMListCtrl::OnSize)
END_EVENT_TABLE()
