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

#include <chmfontdialog.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wx.h>

namespace {

const wxChar* test_page = wxT(
    "<html><body><table><tr><td valign=\"top\">Normal face<br>(and <u>underlined</u>. <i>Italic face.</i> "
    "<b>Bold face.</b> <b><i>Bold italic face.</i></b><br><font size=-2>font size -2</font><br><font size=-1>"
    "font size -1</font><br><font size=+0>font size +0</font><br><font size=+1>font size +1</font><br><font "
    "size=+2>font size +2</font><br><font size=+3>font size +3</font><br><font size=+4>font size +4</font>"
    "</td><td valign=\"top\"><tt>Fixed size face.<br> <b>bold</b> <i>italic</i> <b><i>bold italic <u>"
    "underlined</u></i></b><br><font size=-2>font size -2</font><br><font size=-1>font size -1</font><br>"
    "<font size=+0>font size +0</font><br><font size=+1>font size +1</font><br><font size=+2>font size +2"
    "</font><br><font size=+3>font size +3</font><br><font size=+4>font size +4</font></tt></td></tr></table>"
    "</body></html>");

}

CHMFontDialog::CHMFontDialog(wxWindow* parent, const wxArrayString& normalFonts, const wxArrayString& fixedFonts,
                             const wxString& normalFont, const wxString& fixedFont, int fontSize)
    : wxDialog(parent, -1, _("Change fonts..")), _normalFont(normalFont), _fixedFont(fixedFont), _fontSize(fontSize)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    auto sizer    = new wxFlexGridSizer(2, 3, 2, 5);

    sizer->Add(new wxStaticText(this, -1, _("Normal font:")));
    sizer->Add(new wxStaticText(this, -1, _("Fixed font:")));
    sizer->Add(new wxStaticText(this, -1, _("Font size:")));

    sizer->Add(_normalFControl = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, nullptr,
                                                wxCB_DROPDOWN | wxCB_READONLY));
    sizer->Add(_fixedFControl = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0, nullptr,
                                               wxCB_DROPDOWN | wxCB_READONLY));

    sizer->Add(_fontSizeControl = new wxSpinCtrl(this, -1));

    _fontSizeControl->SetRange(2, 100);

    topsizer->Add(sizer, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    topsizer->Add(new wxStaticText(this, -1, _("Preview:")), 0, wxLEFT | wxTOP, 10);
    topsizer->Add(
        _test = new wxHtmlWindow(this, -1, wxDefaultPosition, wxSize(20, 150), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER),
        1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 10);

    auto sizer2 = new wxBoxSizer(wxHORIZONTAL);
    auto ok     = new wxButton(this, wxID_OK, _("OK"));

    sizer2->Add(ok, 0, wxALL, 10);
    ok->SetDefault();

    sizer2->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 10);
    topsizer->Add(sizer2, 0, wxALIGN_RIGHT);

    SetAutoLayout(true);
    SetSizer(topsizer);
    topsizer->Fit(this);
    Centre(wxBOTH);

    InitDialog(normalFonts, fixedFonts);
}

void CHMFontDialog::UpdatePreview()
{
    if (!_test) // this can happen with wxMSW
        return;

    wxBusyCursor bc;

    _normalFont = _normalFControl->GetStringSelection();
    _fixedFont  = _fixedFControl->GetStringSelection();

    auto size = _fontSizeControl->GetValue();

    for (auto i = -3; i <= 3; ++i)
        _sizes[i + 3] = size + i * 2;

    _test->SetFonts(_normalFont, _fixedFont, _sizes);
    _test->SetPage(test_page);
}

void CHMFontDialog::OnUpdate(wxCommandEvent&)
{
    UpdatePreview();
}

void CHMFontDialog::OnUpdateSpin(wxSpinEvent&)
{
    UpdatePreview();
}

void CHMFontDialog::InitDialog(const wxArrayString& normalFonts, const wxArrayString& fixedFonts)
{
    if (_normalFont.IsEmpty())
        _normalFont = wxFont(_fontSize, wxSWISS, wxNORMAL, wxNORMAL, false).GetFaceName();

    if (_fixedFont.IsEmpty())
        _fixedFont = wxFont(_fontSize, wxMODERN, wxNORMAL, wxNORMAL, false).GetFaceName();

    for (auto&& font : normalFonts)
        _normalFControl->Append(font);

    for (auto&& font : fixedFonts)
        _fixedFControl->Append(font);

    if (!_normalFont.IsEmpty())
        _normalFControl->SetStringSelection(_normalFont);
    else
        _normalFControl->SetSelection(0);

    if (!_fixedFont.IsEmpty())
        _fixedFControl->SetStringSelection(_fixedFont);
    else
        _fixedFControl->SetSelection(0);

    _fontSizeControl->SetValue(_fontSize);

    UpdatePreview();
}

BEGIN_EVENT_TABLE(CHMFontDialog, wxDialog)
EVT_COMBOBOX(-1, CHMFontDialog::OnUpdate)
EVT_SPINCTRL(-1, CHMFontDialog::OnUpdateSpin)
END_EVENT_TABLE()
