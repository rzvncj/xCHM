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


#include <chmlistctrl.h>
#include <chmhtmlwindow.h>
#include <wx/settings.h>


CHMListCtrl::CHMListCtrl(wxWindow *parent, CHMHtmlWindow *html,
			 wxWindowID id)
	: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
		     wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | 
		     wxLC_VIRTUAL | wxSUNKEN_BORDER), _html(html)
{
	InsertColumn(0, wxEmptyString);
	SetItemCount(0);
}


void CHMListCtrl::Reset()
{
	DeleteAllItems();
	SetItemCount(0); // Is this really needed?

	_titles.Clear();
	_urls.Clear();
}


void CHMListCtrl::AddTitle(const wxString& title)
{ 
	_titles.Add(title);
	SetItemCount(_titles.GetCount());
}


void CHMListCtrl::LoadSelected()
{
	long item = -1;
        item = GetNextItem(item,
			   wxLIST_NEXT_ALL,
			   wxLIST_STATE_SELECTED);

        if(item == -1 || item > (long)_urls.GetCount() - 1)
		return;

	_html->AbsoluteFollows(true);
	_html->LoadPage(_urls[(size_t)item]);	
}


void CHMListCtrl::UpdateUI()
{
	SetColumnWidth(0, -1);
	int currSize = GetColumnWidth(0);
	int newSize = GetClientSize().GetWidth() - 1;
	int cpp = GetCountPerPage();

#ifdef __WXGTK__
	// If there's a scrollbar extract the width from the client area.
	if(cpp > 0 && GetItemCount() > GetCountPerPage())
		newSize -= wxSystemSettings::GetSystemMetric(wxSYS_VSCROLL_X);
#endif
		
	SetColumnWidth(0, currSize >= newSize ? currSize : newSize);
}


void CHMListCtrl::OnSize(wxSizeEvent& event)
{
	UpdateUI();
	event.Skip();
}


wxString CHMListCtrl::OnGetItemText(long item, long column) const
{
	if(column || item > (long)_titles.GetCount() - 1)
		return wxEmptyString;

	return _titles[item];
}



BEGIN_EVENT_TABLE(CHMListCtrl, wxListCtrl)
	EVT_SIZE(CHMListCtrl::OnSize)
END_EVENT_TABLE()
