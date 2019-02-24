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

#ifndef __CHMFONTDIALOG_H
#define __CHMFONTDIALOG_H

#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/spinctrl.h>

/*!
  \class wxDialog
  \brief wxWidgets generic dialog class.
*/

//! Custom font chooser dialog class.
class CHMFontDialog : public wxDialog {
public:
    /*!
      \brief Constructs a CHMFontDialog.
      \param parent The parent window.
      \param normalFonts Array of strings denoting all the normal
      font faces' names available on the system. Managed by the caller.
      \param fixedFonts Array of strings denoting all the fixes fonts
      faces' names available on the system. Managed by the caller.
      \param normalFont The normal font currently in use by the caller.
      \param fixedFont The fixed font currently in use by the caller.
      \param fontSize The size of the font currently selected.
     */
    CHMFontDialog(wxWindow* parent, wxArrayString* normalFonts, wxArrayString* fixedFonts, const wxString& normalFont,
                  const wxString& fixedFont, const int fontSize);

    //! Returns the fixed font face name.
    const wxString& FixedFont() const { return _fixedFont; }

    //! Returns the normal font face name.
    const wxString& NormalFont() const { return _normalFont; }

    //! Return the selected font size.
    int* Sizes() { return _sizes; }

protected:
    //! This is called when a font is selected from the combo box.
    void OnUpdate(wxCommandEvent& event);

    //! This is called when you click on the font size spin control.
    void OnUpdateSpin(wxSpinEvent& event);

private:
    //! Helper. Updates the font preview window.
    void UpdatePreview();

    //! Helper. Initializes the dialog with the passed data.
    void InitDialog(wxArrayString* normalFonts, wxArrayString* fixedFonts);

private:
    wxHtmlWindow* _test {nullptr};
    wxSpinCtrl*   _fontSizeControl {nullptr};
    wxComboBox*   _normalFControl {nullptr};
    wxComboBox*   _fixedFControl {nullptr};

    wxString _normalFont;
    wxString _fixedFont;
    int      _sizes[7];
    int      _fontSize;

private:
    DECLARE_EVENT_TABLE();
};

#endif // __CHMFONTDIALOG_H
