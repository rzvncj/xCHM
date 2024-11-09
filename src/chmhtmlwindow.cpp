/*
  Copyright (C) 2003 - 2024  Razvan Cojocaru <rzvncj@gmail.com>
  Mac OS specific patches contributed by Chanler White
  <cawhite@nwrails.com>
  "Save link as" patch contributed by Joerg Wunsch
  <joerg_wunsch@users.sourceforge.net>

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
#include <chminputstream.h>
#include <hhcparser.h>
#include <memory>
#include <wx/clipbrd.h>
#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/wx.h>

class CHMDropTarget : public wxFileDropTarget {

public:
    CHMDropTarget(CHMFrame* frame) : _frame(frame) {}

    bool OnDropFiles(wxCoord /* x */, wxCoord /* y */, const wxArrayString& filenames) override
    {
        if (filenames.size() == 1 && filenames[0].Lower().ends_with(".chm")) {
            _frame->LoadCHM(filenames[0]);
            return true;
        }

        return false;
    }

private:
    CHMFrame* _frame;
};

CHMHtmlWindow::CHMHtmlWindow(wxWindow* parent, wxTreeCtrl* tc, CHMFrame* frame)
    : wxHtmlWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 200)), _tcl(tc), _menu(std::make_unique<wxMenu>()),
      _frame(frame)
{
    _menu->Append(ID_PopupForward, _("For&ward"));
    _menu->Append(ID_PopupBack, _("&Back"));
    _menu->Append(ID_CopyLink, _("Copy &link location"));
    _menu->Append(ID_SaveLinkAs, _("&Save link as.."));
    _menu->Append(ID_OpenInNewTab, _("&Open in a new tab"));

    _menu->AppendSeparator();
    _menu->Append(ID_CopySel, _("&Copy selection"));
    _menu->AppendSeparator();
    _menu->Append(ID_PopupFind, _("&Find in page.."));
    _menu->AppendSeparator();
    _menu->Append(ID_PopupFullScreen, _("&Toggle fullscreen mode"));
    SetDropTarget(new CHMDropTarget(frame));
}

bool CHMHtmlWindow::LoadPage(const wxString& location)
{
    wxLogNull log;
    auto      tmp = location;

    if (!tmp.Left(19).CmpNoCase(wxT("javascript:fullsize")))
        tmp = tmp.AfterFirst(wxT('\'')).BeforeLast(wxT('\''));

    if (_syncTree &&
        // We should be looking for a valid page, not / (home).
        !location.AfterLast(wxT('/')).IsEmpty() && _tcl->GetCount() > 1) {

        wxFileName fwfn(tmp.AfterLast(wxT(':')).BeforeFirst(wxT('#')), wxPATH_UNIX);
        auto       cwd = GetParser()->GetFS()->GetPath().AfterLast(wxT(':'));
        fwfn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE, cwd, wxPATH_UNIX);

        // Sync will call SelectItem() on the tree item if it finds one, and that in turn will call
        // LoadPage() with _syncTree set to false.
        Sync(_tcl->GetRootItem(), fwfn.GetFullPath(wxPATH_UNIX));

        if (_found)
            _found = false;
    }

    Scroll(0, 0);

    return wxHtmlWindow::LoadPage(tmp);
}

void CHMHtmlWindow::Sync(wxTreeItemId root, const wxString& page)
{
    if (_found)
        return;

    auto data = dynamic_cast<URLTreeItem*>(_tcl->GetItemData(root));

    wxString url;

    if (data)
        url = (data->_url).BeforeFirst(wxT('#'));

    if (data && !url.CmpNoCase(page)) {
        _found = true;
        _tcl->SelectItem(root);
        return;
    }

    wxTreeItemIdValue cookie;

    for (auto child = _tcl->GetFirstChild(root, cookie); child; child = _tcl->GetNextChild(root, cookie)) {
        Sync(child, page);
    }
}

wxString CHMHtmlWindow::GetPrefix(const wxString& location) const
{
    return location.AfterLast(wxT(':')).AfterFirst(wxT('/')).BeforeLast(wxT('/'));
}

void CHMHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    auto url = link.GetHref();

    LoadPage(url);

    if (!url.Left(7).CmpNoCase(wxT("MS-ITS:")))
        _frame->UpdateCHMInfo();
}

wxHtmlCell* CHMHtmlWindow::FindFirst(wxHtmlCell* parent, const wxString& word, bool wholeWords, bool caseSensitive)
{
    auto tmp = word;

    if (!parent)
        return nullptr;

    if (!caseSensitive)
        tmp.MakeLower();

    // If this cell is not a container, the for body will never happen (GetFirstChild() will return nullptr).
    for (auto cell = parent->GetFirstChild(); cell; cell = cell->GetNext()) {
        wxHtmlCell* result;
        if ((result = FindFirst(cell, word, wholeWords, caseSensitive)))
            return result;
    }

    wxHtmlSelection ws;
    ws.Set(parent, parent);
    auto text = parent->ConvertToText(&ws);

    if (text.IsEmpty())
        return nullptr;

    if (!caseSensitive)
        text.MakeLower();

    text.Trim(true);
    text.Trim(false);

    auto found = false;

    if (wholeWords && text == tmp)
        found = true;
    else if (!wholeWords && text.Find(tmp.c_str()) != wxNOT_FOUND)
        found = true;

    if (found) {
        delete m_selection;

        m_selection = new wxHtmlSelection();
        m_selection->Set(parent, parent);

        int  y;
        auto cell = parent;

        for (y = 0; cell != nullptr; cell = cell->GetParent())
            y += cell->GetPosY();

        Scroll(-1, y / wxHTML_SCROLL_STEP);
        Refresh();

        return parent;
    }

    return nullptr;
}

wxHtmlCell* CHMHtmlWindow::FindNext(wxHtmlCell* start, const wxString& word, bool wholeWords, bool caseSensitive)
{
    wxHtmlCell* cell;

    if (!start)
        return nullptr;

    for (cell = start; cell; cell = cell->GetNext()) {
        wxHtmlCell* result;
        if ((result = FindFirst(cell, word, wholeWords, caseSensitive)))
            return result;
    }

    cell = start->GetParent();

    while (cell && !cell->GetNext())
        cell = cell->GetParent();

    if (!cell)
        return nullptr;

    return FindNext(cell->GetNext(), word, wholeWords, caseSensitive);
}

void CHMHtmlWindow::ClearSelection()
{
    delete m_selection;
    m_selection = nullptr;
    Refresh();
}

void CHMHtmlWindow::OnCopy(wxCommandEvent&)
{
    CopySelection();
}

void CHMHtmlWindow::OnFind(wxCommandEvent&)
{
    if (!_fdlg) {
        auto p = GetParent();
        while (p->GetParent())
            p = p->GetParent();

        _fdlg = std::make_unique<CHMFindDialog>(p, this);
    }

    _fdlg->CentreOnParent();
    _fdlg->ShowModal();
    _fdlg->SetFocusToTextBox();
    _fdlg->Reset();
}

void CHMHtmlWindow::OnForward(wxCommandEvent&)
{
    HistoryForward();
}

void CHMHtmlWindow::OnBack(wxCommandEvent&)
{
    HistoryBack();
}

void CHMHtmlWindow::OnSize(wxSizeEvent& event)
{
    int x, y;
    GetViewStart(&x, &y);

    wxHtmlWindow::OnSize(event);

    Scroll(x, y);
    event.Skip(false);
}

void CHMHtmlWindow::OnCopyLink(wxCommandEvent&)
{
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(_link));
        wxTheClipboard->Close();
    }
}

void CHMHtmlWindow::OnSaveLinkAs(wxCommandEvent&)
{
    std::unique_ptr<wxFSFile> f(m_FS->OpenFile(_link));

    if (!f) {
        wxMessageBox(_("OpenFile(") + _link + _(") failed"), _("Error"), wxOK, this);
        return;
    }

    wxFileName wfn(_link);
    auto       suggestedName = wfn.GetFullName();

    if (suggestedName.IsEmpty())
        suggestedName = wxT("index.html");

    auto filename = wxFileSelector(_("Save as"), wxT(""), suggestedName, wxT(""), wxT("*.*"),
                                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this);

    if (!filename.empty()) {
        auto               s = f->GetStream();
        wxFileOutputStream out(filename);

        if (!out.IsOk())
            wxMessageBox(_("Error creating file ") + filename, _("Error"), wxOK, this);
        else {
            char buffer[4096];

            while (!s->Eof()) {
                s->Read(buffer, sizeof(buffer));
                auto nbytes = s->LastRead();
                out.Write(static_cast<void*>(buffer), nbytes);
            }

            wxMessageBox(_("Saved file ") + filename, _("Success"), wxOK, this);
        }
    }
}

void CHMHtmlWindow::OnRightClick(wxMouseEvent& event)
{
    if (IsSelectionEnabled())
        _menu->Enable(ID_CopySel, m_selection);

    _menu->Enable(ID_PopupForward, HistoryCanForward());
    _menu->Enable(ID_PopupBack, HistoryCanBack());
    _menu->Enable(ID_CopyLink, false);
    _menu->Enable(ID_SaveLinkAs, false);
    _menu->Enable(ID_OpenInNewTab, false);

    int x, y;
    CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);

    auto cell = GetInternalRepresentation()->FindCellByPos(x, y);

    wxHtmlLinkInfo* linfo = nullptr;

    if (cell)
        linfo = cell->GetLink();

    if (linfo) {
        _link = linfo->GetHref();
        _menu->Enable(ID_CopyLink, true);
        _menu->Enable(ID_SaveLinkAs, true);
        _menu->Enable(ID_OpenInNewTab, true);
    }

    PopupMenu(_menu.get(), event.GetPosition());
}

void CHMHtmlWindow::OnOpenInNewTab(wxCommandEvent&)
{
    auto link = _link;

    if (link.StartsWith(wxT("#"))) // anchor
        link = GetOpenedPage() + _link;

    _frame->AddHtmlView(GetParser()->GetFS()->GetPath(), link);
}

void CHMHtmlWindow::OnToggleFullScreen(wxCommandEvent&)
{
    _frame->ToggleFullScreen();
}

void CHMHtmlWindow::OnChar(wxKeyEvent& event)
{
    int       xStart, yStart, xUnit, yUnit, xSize, ySize;
    bool      skip    = true;
    const int OVERLAP = 3;

    GetViewStart(&xStart, &yStart);
    GetScrollPixelsPerUnit(&xUnit, &yUnit);
    GetClientSize(&xSize, &ySize);

    switch (event.GetKeyCode()) {
    case WXK_SPACE:
    case WXK_PAGEDOWN:
        Scroll(xStart, yStart + ySize / yUnit - OVERLAP);
        skip = false;
        break;
    case WXK_PAGEUP:
    case WXK_BACK:
        Scroll(xStart, yStart - ySize / yUnit + OVERLAP);
        skip = false;
        break;
    case WXK_ESCAPE:
        _frame->ToggleFullScreen(true);
        break;
    case 'g':
    case WXK_HOME:
        Scroll(0, 0);
        break;
    case WXK_END:
    case 'G': {
        int x, y;

        GetVirtualSize(&x, &y);
        Scroll(0, y / yUnit);
        break;
    }
    case 'j':
        event.m_keyCode = WXK_DOWN;
        break;
    case 'k':
        event.m_keyCode = WXK_UP;
        break;
    case 'h':
        event.m_keyCode = WXK_LEFT;
        break;
    case 'l':
        event.m_keyCode = WXK_RIGHT;
        break;
    default:
        break;
    }

    if (skip)
        event.Skip();
}

void CHMHtmlWindow::OnSetTitle(const wxString& title)
{
    auto parent = dynamic_cast<CHMHtmlNotebook*>(GetParent());

    if (parent)
        parent->SetPageText(parent->GetSelection(), title);

    wxHtmlWindow::OnSetTitle(title);
}

BEGIN_EVENT_TABLE(CHMHtmlWindow, wxHtmlWindow)
EVT_MENU(ID_CopySel, CHMHtmlWindow::OnCopy)
EVT_MENU(ID_PopupFind, CHMHtmlWindow::OnFind)
EVT_MENU(ID_PopupForward, CHMHtmlWindow::OnForward)
EVT_MENU(ID_PopupBack, CHMHtmlWindow::OnBack)
EVT_MENU(ID_CopyLink, CHMHtmlWindow::OnCopyLink)
EVT_MENU(ID_SaveLinkAs, CHMHtmlWindow::OnSaveLinkAs)
EVT_MENU(ID_OpenInNewTab, CHMHtmlWindow::OnOpenInNewTab)
EVT_MENU(ID_PopupFullScreen, CHMHtmlWindow::OnToggleFullScreen)
EVT_CHAR(CHMHtmlWindow::OnChar)
EVT_RIGHT_DOWN(CHMHtmlWindow::OnRightClick)
EVT_SIZE(CHMHtmlWindow::OnSize)
END_EVENT_TABLE()
