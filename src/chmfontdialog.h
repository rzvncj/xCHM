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

#include <wx/html/htmlwin.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
#include <wx/combobox.h>


#define CHM_DEFAULT_FONT_SIZE 14


class CHMFontDialog : public wxDialog
{
public:
	CHMFontDialog(wxWindow *parent, wxArrayString *normalFonts,
		      wxArrayString *fixedFonts);

	const wxString& FixedFont() { return _fixedFont; }
	const wxString& NormalFont() { return _normalFont; }
	int* Sizes() { return _sizes; }

protected:
	void OnUpdate(wxCommandEvent& event);
	void OnUpdateSpin(wxSpinEvent& event);
	void UpdatePreview();

private:
	void InitDialog(wxArrayString *normalFonts, wxArrayString *fixedFonts);
	
private:
	wxHtmlWindow *_test;
	wxString _fixedFont;
	wxString _normalFont;

	wxSpinCtrl *_fontSize;
	wxComboBox *_normalFControl;
	wxComboBox *_fixedFControl;
	int _sizes[7];

	DECLARE_EVENT_TABLE();
};

