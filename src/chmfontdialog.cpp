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

#include <chmfontdialog.h>
#include <chmframe.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/wx.h>

namespace {

const wxChar* test_page = wxT(R"(
<html>

<body>
    <table>
        <tr>
            <td valign="top">Normal face<br>(and <u>underlined</u>. <i>Italic face.</i> <b>Bold face.</b> <b><i>Bold
                        italic face.</i></b><br>
                <font size=-2>font size -2</font><br>
                <font size=-1>font size -1</font><br>
                <font size=+0>font size +0</font><br>
                <font size=+1>font size +1</font><br>
                <font size=+2>font size +2</font><br>
                <font size=+3>font size +3</font><br>
                <font size=+4>font size +4</font>
            </td>
            <td valign="top"><tt>Fixed size face.<br> <b>bold</b> <i>italic</i> <b><i>bold italic
                            <u>underlined</u></i></b><br>
                    <font size=-2>font size -2</font><br>
                    <font size=-1>font size -1</font><br>
                    <font size=+0>font size +0</font><br>
                    <font size=+1>font size +1</font><br>
                    <font size=+2>font size +2</font><br>
                    <font size=+3>font size +3</font><br>
                    <font size=+4>font size +4</font>
                </tt></td>
        </tr>
    </table>
</body>

</html>)");

}

CHMFontDialog::CHMFontDialog(wxWindow* parent, const wxString& normalFont, const wxString& fixedFont, int fontSize)
    : wxDialog(parent, wxID_ANY, _("Change fonts..")),
      _normalFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, normalFont),
      _fixedFont(fontSize, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fixedFont)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    auto sizer    = new wxFlexGridSizer(2, 2, 2, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, _("Normal font:")));
    sizer->Add(new wxStaticText(this, wxID_ANY, _("Fixed font:")));

    sizer->Add(_normalFControl = new wxFontPickerCtrl(this, wxID_ANY, _normalFont, wxDefaultPosition, wxSize(200, -1)));
    sizer->Add(_fixedFControl = new wxFontPickerCtrl(this, wxID_ANY, _fixedFont, wxDefaultPosition, wxSize(200, -1)));

    topsizer->Add(sizer, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    topsizer->Add(new wxStaticText(this, wxID_ANY, _("Preview:")), 0, wxLEFT | wxTOP, 10);
    topsizer->Add(_test = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(20, 150),
                                           wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER),
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

    UpdatePreview();
}

void CHMFontDialog::UpdatePreview()
{
    if (!_test) // this can happen with wxMSW
        return;

    wxBusyCursor bc;

    auto parent = dynamic_cast<CHMFrame*>(GetParent());
    auto sizes  = parent->ComputeFontSizes(_normalFont.GetPointSize());

    _test->SetFonts(_normalFont.GetFaceName(), _fixedFont.GetFaceName(), sizes.data());
    _test->SetPage(test_page);
}

void CHMFontDialog::OnUpdate(wxFontPickerEvent& event)
{
    _normalFont = _normalFControl->GetSelectedFont();
    _fixedFont  = _fixedFControl->GetSelectedFont();

    _normalFont.SetPointSize(event.GetFont().GetPointSize());
    _fixedFont.SetPointSize(event.GetFont().GetPointSize());

    UpdatePreview();
}

BEGIN_EVENT_TABLE(CHMFontDialog, wxDialog)
EVT_FONTPICKER_CHANGED(wxID_ANY, CHMFontDialog::OnUpdate)
END_EVENT_TABLE()
