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


#ifndef __CHMINDEXPANEL_H_
#define __CHMINDEXPANEL_H_


#include <chmhtmlwindow.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>


//! IDs for various widget events.
enum {
	ID_SearchIndex = 1500,
	ID_IndexList,
};


class CHMIndexPanel : public wxPanel {

public:
	CHMIndexPanel(wxWindow *parent, CHMHtmlWindow* html);
	~CHMIndexPanel();

public:
	wxListBox* GetIndexList() const { return _list; }
	wxArrayString* GetUrlArray() { return &_listUrls; }

	void Reset();

	//! Sets the font.
	void SetNewFont(const wxFont& font);

protected:
	//! This gets called when the user clicks on a list item.
	void OnIndexSel(wxCommandEvent& event);

	void OnText(wxCommandEvent& event);

private:
	CHMHtmlWindow* _html;
	wxListBox* _list;
	wxTextCtrl* _text;
	wxArrayString _listUrls;

private:
	DECLARE_EVENT_TABLE()
};


#endif // __CHMINDEXPANEL_H_

