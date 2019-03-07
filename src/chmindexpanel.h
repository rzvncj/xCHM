/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>

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

#ifndef __CHMINDEXPANEL_H_
#define __CHMINDEXPANEL_H_

#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/textctrl.h>

class CHMHtmlNotebook;
class CHMListCtrl;

/*!
  \class wxPanel
  \brief generic wxWidgets panel widget class.
*/

//! IDs for various widget events.
enum {
    ID_SearchIndex = 1500,
    ID_IndexClicked,
};

//! Custom panel for displaying the .chm index (if available).
class CHMIndexPanel : public wxPanel {

public:
    /*!
      \brief Initializes the panel.
      \param parent Parent widget.
      \param nbhtml HTML-capable widget used for displaying pages
      from the index.
     */
    CHMIndexPanel(wxWindow* parent, CHMHtmlNotebook* nbhtml);

public:
    //! Accesor for the CHMListCtrl used by this panel.
    CHMListCtrl* GetResultsList() { return _lc; }

    //! Clears the textbox and removes all items from the list control.
    void Reset();

    //! Sets the font.
    void SetNewFont(const wxFont& font);

protected:
    //! This gets called when the user clicks on a list item.
    void OnIndexSel(wxListEvent& event);

    //! This gets called when the user presses enter on a list item.
    void OnIndexSelRet(wxCommandEvent& event);

    //! Called whenever the user types a letter in the textbox.
    void OnText(wxCommandEvent& event);

private:
    wxTextCtrl*      _text;
    CHMListCtrl*     _lc {nullptr};
    bool             _navigate {true};

private:
    DECLARE_EVENT_TABLE()
};

#endif // __CHMINDEXPANEL_H_
