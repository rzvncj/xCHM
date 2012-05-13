/*

  Copyright (C) 2003 - 2012  Razvan Cojocaru <razvanco@gmx.net>
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


#include <chmlistctrl.h>
#include <chmhtmlnotebook.h>
#include <chminputstream.h>
#include <wx/settings.h>


#define INDEX_HINT_SIZE 2048


// Helper

int CompareItemPairs(CHMListPairItem *item1, CHMListPairItem *item2)
{
	return (item1->_title).CmpNoCase(item2->_title);
}



// CHMListCtrl implementation

CHMListCtrl::CHMListCtrl(wxWindow *parent, CHMHtmlNotebook *nbhtml,
			 wxWindowID id)
	: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
		     wxLC_VIRTUAL |
		     wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | 
		     wxLC_SORT_ASCENDING | wxSUNKEN_BORDER), 
	  _items(CompareItemPairs),
	  _nbhtml(nbhtml)
{
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


void CHMListCtrl::AddPairItem(const wxString& title, const wxString& url)
{
	_items.Add(new CHMListPairItem(title, url));
	SetItemCount(_items.GetCount());
}


void CHMListCtrl::LoadSelected()
{
	long item = -1;
        item = GetNextItem(item,
			   wxLIST_NEXT_ALL,
			   wxLIST_STATE_SELECTED);

        if(item == -1 || item > (long)_items.GetCount() - 1)
		return;

	CHMFile *chmf = CHMInputStream::GetCache();
	
	if(chmf) {
		wxString fname = _items[item]->_url;

		if(!fname.StartsWith(wxT("file:")))
			fname = wxString(wxT("file:")) + chmf->ArchiveName()
				+ wxT("#xchm:/") + _items[item]->_url;

		_nbhtml->LoadPageInCurrentView(fname);
	}
}



void CHMListCtrl::UpdateUI()
{
	SetColumnWidth(0, GetClientSize().GetWidth());
}


void CHMListCtrl::FindBestMatch(const wxString& title)
{
	wxListItem info;
	info.m_col = 0;

	long sz = GetItemCount();
	int tl = title.length();

	for(long i = 0; i < sz; ++i) {

		info.m_itemId = i;
		GetItem(info);

		if(!info.m_text.Left(tl).CmpNoCase(title)) {

			EnsureVisible(i);
			SetItemState(i, wxLIST_STATE_SELECTED,
				     wxLIST_STATE_SELECTED);
			break;
		}
	}

	Refresh();
	wxWindow::Update();
}


void CHMListCtrl::ResetItems()
{
	for(long i = 0; i < (long)_items.GetCount(); ++i) {
		if(_items[i] != NULL)
			delete _items[i];
	}
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
	if(column != 0 || item == -1 || item > (long)_items.GetCount() - 1)
		return wxT("");

	return _items[item]->_title;
}


BEGIN_EVENT_TABLE(CHMListCtrl, wxListCtrl)
	EVT_SIZE(CHMListCtrl::OnSize)
END_EVENT_TABLE()


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

