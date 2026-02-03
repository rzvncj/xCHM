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

#ifndef __CHMFONTDIALOG_H
#define __CHMFONTDIALOG_H

#include <wx/dialog.h>
#include <wx/fontpicker.h>
#include <wx/html/htmlwin.h>

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
      \param normalFont The normal font currently in use by the caller.
      \param fixedFont The fixed font currently in use by the caller.
      \param fontSize The size of the font currently selected.
     */
    CHMFontDialog(wxWindow* parent, const wxString& normalFont, const wxString& fixedFont, int fontSize);

    //! Returns the fixed font face name.
    wxString FixedFont() const { return _fixedFont.GetFaceName(); }

    //! Return the selected font size.
    int FontSize() const { return _normalFont.GetPointSize(); }
    //! Returns the normal font face name.
    wxString NormalFont() const { return _normalFont.GetFaceName(); }

protected:
    //! This is called when a font is selected from the combo box.
    void OnUpdate(wxFontPickerEvent& event);

private:
    //! Helper. Updates the font preview window.
    void UpdatePreview();

private:
    wxHtmlWindow*     _test {nullptr};
    wxFontPickerCtrl* _normalFControl {nullptr};
    wxFontPickerCtrl* _fixedFControl {nullptr};

    wxFont _normalFont;
    wxFont _fixedFont;

private:
    DECLARE_EVENT_TABLE();
};

#endif // __CHMFONTDIALOG_H
