/*
  Copyright (C) 2003 - 2026  Razvan Cojocaru <razvanc@mailbox.org>
  Tabbed browsing support developed by Cedric Boudinet <bouced@gmx.fr>
  (this file originally written by Cedric Boudinet)

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

#include <chmframe.h>
#include <chmhtmlnotebook.h>
#include <chmhtmlwindow.h>

CHMHtmlNotebook::CHMHtmlNotebook(wxWindow* parent, wxTreeCtrl* tc, const wxString& normalFont,
                                 const wxString& fixedFont, int fontSize, CHMFrame* frame)
    : wxAuiNotebook(parent), _tcl(tc), _frame(frame), _fontsNormalFace(normalFont), _fontsFixedFace(fixedFont),
      _fontSize(fontSize)
{
    wxAcceleratorEntry entries[] = {{wxACCEL_CTRL, WXK_PAGEUP, ID_PriorPage},
                                    {wxACCEL_CTRL, WXK_PAGEDOWN, ID_NextPage},
                                    {wxACCEL_CTRL, '=', ID_ZoomIn},
                                    {wxACCEL_CTRL, '-', ID_ZoomOut}};

    wxAcceleratorTable accel(sizeof(entries) / sizeof(wxAcceleratorEntry), entries);
    SetAcceleratorTable(accel);

    AddHtmlView(wxEmptyString, wxT("memory:about.html"));
}

CHMHtmlWindow* CHMHtmlNotebook::CreateView()
{
    auto htmlWin = new CHMHtmlWindow(this, _tcl, _frame);
    auto sizes   = _frame->ComputeFontSizes(_fontSize);

    htmlWin->SetRelatedStatusBar(_frame->GetStatusBar());
    htmlWin->SetFonts(_fontsNormalFace, _fontsFixedFace, sizes.data());

    AddTab(htmlWin, _("(Empty page)"));
    SetSelection(GetPageCount() - 1);

    return htmlWin;
}

void CHMHtmlNotebook::AddHtmlView(const wxString& path, const wxString& link)
{
    auto htmlWin = CreateView();

    if (htmlWin && !link.IsEmpty()) {
        htmlWin->GetParser()->GetFS()->ChangePathTo(path);
        htmlWin->LoadPage(link);
    }
}

bool CHMHtmlNotebook::LoadPageInCurrentView(const wxString& location)
{
    return GetCurrentPage()->LoadPage(location);
}

CHMHtmlWindow* CHMHtmlNotebook::GetCurrentPage()
{
    auto selection = GetSelection();

    if (selection == wxNOT_FOUND)
        return CreateView();

    return dynamic_cast<CHMHtmlWindow*>(wxAuiNotebook::GetPage(selection));
}

void CHMHtmlNotebook::OnGoToNextPage(wxCommandEvent&)
{
    auto selection = GetSelection();

    if (selection >= static_cast<decltype(selection)>(GetPageCount() - 1))
        return;

    SetSelection(selection + 1);
}

void CHMHtmlNotebook::OnGoToPriorPage(wxCommandEvent&)
{
    auto selection = GetSelection();

    if (selection <= 0)
        return;

    SetSelection(selection - 1);
}

void CHMHtmlNotebook::OnCloseTab(wxAuiNotebookEvent&)
{
    if (!GetPageCount())
        _frame->Close(true);
}

void CHMHtmlNotebook::OnNewTab(wxCommandEvent&)
{
    AddHtmlView(wxEmptyString, wxEmptyString);
}

void CHMHtmlNotebook::OnZoomIn(wxCommandEvent&)
{
    SetChildrenFonts(_fontsNormalFace, _fontsFixedFace, _fontSize + 1);
}

void CHMHtmlNotebook::OnZoomOut(wxCommandEvent&)
{
    SetChildrenFonts(_fontsNormalFace, _fontsFixedFace, _fontSize - 1);
}

void CHMHtmlNotebook::CloseAllPagesExceptFirst()
{
    SetSelection(0);

    while (GetPageCount() > 1)
        DeletePage(1);
}

void CHMHtmlNotebook::SetChildrenFonts(const wxString& normalFace, const wxString& fixedFace, int fontSize)
{
    _fontsNormalFace = normalFace;
    _fontsFixedFace  = fixedFace;
    _fontSize        = fontSize;

    auto nPageCount = GetPageCount();
    auto sizes      = _frame->ComputeFontSizes(_fontSize);

    for (decltype(nPageCount) nPage = 0; nPage < nPageCount; ++nPage) {
        auto chw = dynamic_cast<CHMHtmlWindow*>(GetPage(nPage));

        if (chw)
            chw->SetFonts(_fontsNormalFace, _fontsFixedFace, sizes.data());
    }
}

bool CHMHtmlNotebook::AddTab(wxWindow* page, const wxString& title)
{
    if (!page)
        return false;

    return AddPage(page, title);
}

BEGIN_EVENT_TABLE(CHMHtmlNotebook, wxAuiNotebook)
EVT_MENU(ID_PriorPage, CHMHtmlNotebook::OnGoToPriorPage)
EVT_MENU(ID_NextPage, CHMHtmlNotebook::OnGoToNextPage)
EVT_MENU(ID_ZoomIn, CHMHtmlNotebook::OnZoomIn)
EVT_MENU(ID_ZoomOut, CHMHtmlNotebook::OnZoomOut)
EVT_AUINOTEBOOK_PAGE_CLOSED(wxID_ANY, CHMHtmlNotebook::OnCloseTab)
END_EVENT_TABLE()
