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


#ifndef __CHMSEARCHPANEL_HPP_
#define __CHMSEARCHPANEL_HPP_


#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/listbox.h>


enum {
	ID_SearchText = 1024,
	ID_SearchButton,
	ID_Results,
};


class CHMSearchPanel : public wxPanel {

public:
	CHMSearchPanel(wxWindow *parent, wxTreeCtrl *topics);

	void Reset();

private:
	wxTreeCtrl* _tcl;
	wxTextCtrl* _text;
	wxCheckBox* _case;
	wxCheckBox* _whole;
	wxButton* _search;
	wxListBox* _results;
};


#endif // __CHMSEARCHPANEL_HPP_

