/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>
   XML-RPC/Context ID code contributed by Eamon Millman / PCI Geomatics
  <millman@pcigeomatics.com>
  Tree control icons code (and the icons) contributed by Fritz Elfert
  <felfert@users.sourceforge.net>.

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

#include <algorithm>
#include <chmfontdialog.h>
#include <chmframe.h>
#include <chmhtmlnotebook.h>
#include <chmhtmlwindow.h>
#include <chmindexpanel.h>
#include <chminputstream.h>
#include <chmlistctrl.h>
#include <chmsearchpanel.h>
#include <hhcparser.h>
#include <wx/accel.h>
#include <wx/bitmap.h>
#include <wx/busyinfo.h>
#include <wx/filesys.h>
#include <wx/fontenum.h>
#include <wx/fs_mem.h>
#include <wx/imaglist.h>
#include <wx/mimetype.h>
#include <wx/statbox.h>
#include <wx/utils.h>

#define OPEN_HELP _("Open a CHM book.")
#define FONTS_HELP _("Change fonts.")
#define PRINT_HELP _("Print the page currently displayed.")
#define CONTENTS_HELP _("Toggle table of contents.")
#define HOME_HELP _("Go to the book's start page.")
#define FORWARD_HELP _("Go forward in history. Per book.")
#define BACK_HELP _("Back to the last visited page. Per book.")
#define ABOUT_HELP _("About the program.")
#define COPY_HELP _("Copy selection.")
#define FIND_HELP _("Find word in page.")
#define FULLSCREEN_HELP _("Toggle fullscreen mode.")
#define CLOSETAB_HELP _("Close the current tab")
#define NEWTAB_HELP _("Open a new tab")
#define REGISTER_EXTENSION_HELP _("Associate the .chm file extension with xCHM.")

namespace {

const wxChar* greeting = wxT(
    "<html><head><title>About</title></head><body><table border=0><tr><td align=\"left\">"
    "<img src=\"memory:logo.xpm\"></td><td align=\"left\">Hello, and welcome to <B>xCHM</B>, the UNIX CHM viewer."
    "<br><br><B>xCHM</B> has been written by Razvan Cojocaru (rzvncj@gmail.com). It is licensed under the "
    "<TT>GPL</TT>.<br>It's based on Jed Wing's <a href=\"http://www.jedrea.com/chmlib/\">CHMLIB</a> and "
    "<a href=\"http://www.wxwidgets.org\">wxWidgets</a>.<br><br></td></tr></table>"
    "<br>If you'd like to know more about CHM, go to <a href=\"http://www.nongnu.org/chmspec/latest/\">Pabs' CHM"
    " Specification page</a>.<br>Pabs has contributed time and knowledge to the development of <B>xCHM</B>, "
    "and features such as the fast index search would not have been implemented without his help."
    " Portions of the fast index search are modified versions of Pabs' <TT>GPL</TT>d <TT>chmdeco</TT> code."
    "<br><br>If you'd like to use the code in your own stuff please figure <TT>GPL</TT> out first."
#if !defined(wxUSE_UNICODE) || !wxUSE_UNICODE
    "<br><br><B>WARNING</B>: your <B>xCHM</B> binary is linked against a non-Unicode version of wxWidgets! "
    "While it will work with most CHMs, it might not properly display special character languages."
#endif
    "<br><br>Tips:<br><ul><li>The global search is an \'AND\' search, i.e. searching for \'word1 word2\' "
    "(quotes not included) will find all the pages that contain both word1 and word2, in whatever order.</li>"
    "<li>Allowing partial matches will find pages that contain words that only start with the typed strings, "
    "i.e. searching for \'ans 4\' (quotes not included) will find pages that contain the sentence "
    "\'The answer is 42.\'.</li><li>Right clicking on the displayed page brings out a popup menu with "
    "common options.</li></ul><br><br>Ctrl-'c' is copy, Ctrl-'f' is find in page, Ctrl-'=' is zoom-in, "
    "Ctrl-'-' is zoom-out.<br><br>Enjoy.</body></html>");

const wxChar* error_page = wxT("<html><body>Error loading CHM file!</body></html>");

const wxChar* about_txt = wxT("xCHM v. " VERSION
                              "\nby Razvan Cojocaru <rzvncj@gmail.com>\n\n"
                              "With thanks to Pabs. Based on Jed Wing's CHMLIB.\n"
                              "XMLRPC code for context sensitive help contributed by Eamon Millman. "
                              "<SPAN> tag support and contents tree icons contributed by Fritz Elfert. "
                              "Tabbed browsing support contributed by Cedric Boudinet. "
                              "Mac builds, suggestions and fixes contributed by Mojca Miklavec. "
                              "Glasses logo by Anca Macinic.");

#include <logo.xpm>
#include <xchm-32.xpm>

} // namespace

CHMFrame::CHMFrame(const wxString& title, const wxString& booksDir, const wxPoint& pos, const wxSize& size,
                   const wxString& normalFont, const wxString& fixedFont, int fontSize, int sashPosition,
                   const wxString& fullAppPath, bool loadTopics, bool loadIndex)
    : wxFrame(nullptr, -1, title, pos, size), _openPath(booksDir), _normalFont(normalFont), _fixedFont(fixedFont),
      _sashPos(sashPosition), _fullAppPath(fullAppPath), _loadTopics(loadTopics), _loadIndex(loadIndex)
{
#if wxUSE_ACCEL
    wxAcceleratorEntry entries[]
        = {{wxACCEL_CTRL, 'F', ID_FindInPage}, {wxACCEL_CTRL, 'C', ID_CopySelection}, {wxACCEL_CTRL, ']', ID_Forward},
           {wxACCEL_CTRL, '[', ID_Back},       {wxACCEL_CTRL, WXK_F4, ID_CloseTab},   {wxACCEL_CTRL, 'Q', ID_Quit}};

    wxAcceleratorTable accel(sizeof(entries) / sizeof(wxAcceleratorEntry), entries);
    SetAcceleratorTable(accel);
#endif

#ifdef __WXMAC__
    wxApp::s_macAboutMenuItemId = ID_About;
#endif

    wxLogNull wln;

    SetIcon(wxIcon(xchm_32_xpm));
    SetMenuBar(CreateMenu());

    _tb = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT
#if defined __WXMSW__ || defined __WXGTK__
                        | wxTB_FLAT | wxTB_DOCKABLE
#endif
    );
    InitToolBar(_tb);

    CreateStatusBar();
    SetStatusText(_("Ready."));

    wxBitmap bitmap(logo_xpm);
    wxMemoryFSHandler::AddFile(wxT("logo.xpm"), bitmap, wxBITMAP_TYPE_XPM);
    wxMemoryFSHandler::AddFile(wxT("about.html"), greeting);
    wxMemoryFSHandler::AddFile(wxT("error.html"), error_page);

    _ep = std::make_unique<wxHtmlEasyPrinting>(wxT("Printing"), this);

    _sw = new wxSplitterWindow(this);
    _sw->SetMinimumPaneSize(CONTENTS_MARGIN);

    _nb = new wxNotebook(_sw, ID_Notebook);
    _nb->Show(false);

    auto contents = CreateContentsPanel();
    _nbhtml       = new CHMHtmlNotebook(_sw, _tcl, _normalFont, _fixedFont, fontSize, this);
    _nbhtml->SetChildrenFonts(_normalFont, _fixedFont, fontSize);

    _csp  = new CHMSearchPanel(_nb, _tcl, _nbhtml);
    _font = _tcl->GetFont();

    _cip = new CHMIndexPanel(_nb, _nbhtml);

    _nb->AddPage(contents, _("Contents"));
    _nb->AddPage(_cip, _("Index"));
    _nb->AddPage(_csp, _("Search"));

    _sw->Initialize(_nbhtml);
    _nbhtml->GetCurrentPage()->SetFocusFromKbd();
}

CHMFrame::~CHMFrame()
{
    // Supposedly, workaround for wxWin
    _tcl->Unselect();
}

void CHMFrame::OnQuit(wxCommandEvent&)
{
    Close(true);
}

void CHMFrame::OnAbout(wxCommandEvent&)
{
    wxMessageBox(about_txt, _("About xCHM"), wxOK | wxICON_INFORMATION, this);
}

void CHMFrame::OnOpen(wxCommandEvent&)
{
    auto selection = wxFileSelector(_("Choose a file.."), _openPath, wxEmptyString, wxT("chm"),
                                    wxT("CHM files (*.chm)|*.chm;*.CHM|") wxT("All files (*.*)|*.*"),
                                    wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (selection.IsEmpty())
        return;

    _openPath = selection.BeforeLast(wxT('/'));
    LoadCHM(selection);
}

std::unique_ptr<wxArrayString> CHMFrame::SortedFontFaceNames(bool fixed) const
{
    wxFontEnumerator enu;
    enu.EnumerateFacenames(wxFONTENCODING_SYSTEM, fixed);

    auto fonts = std::make_unique<wxArrayString>(enu.GetFacenames());
    fonts->Sort();

    return fonts;
}

void CHMFrame::OnChangeFonts(wxCommandEvent&)
{
    wxLogNull wln;

    if (!_normalFonts)
        _normalFonts = SortedFontFaceNames();

    if (!_fixedFonts)
        _fixedFonts = SortedFontFaceNames(true);

    CHMFontDialog cfd(this, *_normalFonts, *_fixedFonts, _normalFont, _fixedFont, _nbhtml->FontSize());

    if (cfd.ShowModal() == wxID_OK) {
        wxBusyCursor bc;

        _nbhtml->SetChildrenFonts(_normalFont = cfd.NormalFont(), _fixedFont = cfd.FixedFont(), cfd.FontSize());

        auto page = _nbhtml->GetCurrentPage()->GetOpenedPage();

        if (page.IsEmpty())
            _nbhtml->GetCurrentPage()->LoadPage(wxT("memory:about.html"));
    }
}

void CHMFrame::OnHome(wxCommandEvent&)
{
    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    _nbhtml->LoadPageInCurrentView(wxT("file:") + chmf->ArchiveName() + wxT("#xchm:") + chmf->HomePage());
}

void CHMFrame::OnHistoryForward(wxCommandEvent&)
{
    _nbhtml->GetCurrentPage()->HistoryForward();
}

void CHMFrame::OnHistoryBack(wxCommandEvent&)
{
    if (_nbhtml->GetCurrentPage()->HistoryCanBack())
        _nbhtml->GetCurrentPage()->HistoryBack();
}

void CHMFrame::OnShowContents(wxCommandEvent&)
{
    if (_sw->IsSplit()) {
        _tb->ToggleTool(ID_Contents, false);
        _menuFile->Check(ID_Contents, false);
        _sashPos = _sw->GetSashPosition();

        _sw->Unsplit(_nb);
        _nb->Show(false);
    } else {
        _tb->ToggleTool(ID_Contents, true);
        _menuFile->Check(ID_Contents, true);

        _nb->Show(true);
        _sw->SplitVertically(_nb, _nbhtml, _sashPos);
    }
}

#if defined(__WXMSW__) || defined(__WXMAC__)

#define EC_WARNING_MSG \
    "This is experimental code, and \
doing this will overwrite any previously registered CHM viewer \
associations.\n\nAre you sure you know what you're doing?"

void CHMFrame::OnRegisterExtension(wxCommandEvent&)
{
    auto answer = wxMessageBox(_(EC_WARNING_MSG), _("Confirm"), wxOK | wxCANCEL, this);

    if (answer == wxOK) {
        // Using NULL instead of nullptr here because of this:
        // error: incomplete definition of type 'wxFormatStringSpecifier<nullptr_t>'
        // which happens on Mac builds with wxWidgets 3.0.
        wxFileTypeInfo fti_xchm(wxT("application/x-chm"), _fullAppPath, wxEmptyString, wxT("Compiled HTML help"),
                                wxT("chm"), NULL);

        wxFileTypeInfo fti_msh(wxT("application/vnd.ms-htmlhelp"), _fullAppPath, wxEmptyString,
                               wxT("Compiled HTML help"), wxT("chm"), NULL);

        auto ft_xchm = wxTheMimeTypesManager->Associate(fti_xchm);
        auto ft_msh  = wxTheMimeTypesManager->Associate(fti_msh);

        if (ft_xchm && ft_msh) {
            ft_xchm->SetDefaultIcon(_fullAppPath);
            ft_msh->SetDefaultIcon(_fullAppPath);
            wxMessageBox(_("Registration successful!"), _("Done"), wxOK, this);
        }
    }
}

#endif // __WXMSW__

void CHMFrame::OnPrint(wxCommandEvent&)
{
    wxLogNull wln;

    auto sizes = ComputeFontSizes(_nbhtml->FontSize());
    _ep->SetFonts(_normalFont, _fixedFont, sizes.data());

    _ep->PrintFile(_nbhtml->GetCurrentPage()->GetOpenedPage());
}

void CHMFrame::OnHistFile(wxCommandEvent& event)
{
    auto f = _fh.GetHistoryFile(event.GetId() - wxID_FILE1);

    if (!f.IsEmpty())
        LoadCHM(f);
}

void CHMFrame::OnFind(wxCommandEvent& event)
{
    _nbhtml->GetCurrentPage()->OnFind(event);
}

void CHMFrame::OnCloseTab(wxCommandEvent&)
{
    _nbhtml->DeletePage(_nbhtml->GetSelection());

    if (!_nbhtml->GetPageCount())
        Close(true);
}

void CHMFrame::OnNewTab(wxCommandEvent& event)
{
    _nbhtml->OnNewTab(event);
}

void CHMFrame::OnCopySelection(wxCommandEvent& event)
{
    _nbhtml->GetCurrentPage()->OnCopy(event);
}

void CHMFrame::OnToggleFullScreen(wxCommandEvent&)
{
    _fullScreen = !_fullScreen;
    ShowFullScreen(_fullScreen, wxFULLSCREEN_ALL);
}

void CHMFrame::OnAddBookmark(wxCommandEvent&)
{
    auto id = _tcl->GetSelection();

    if (!id.IsOk())
        return;

    auto title = _tcl->GetItemText(id);

    if (title.IsEmpty())
        return;

    auto data = dynamic_cast<URLTreeItem*>(_tcl->GetItemData(id));

    if (!data || (data->_url).IsEmpty())
        return;

    _cb->Append(title, new wxString(data->_url));

    _bookmarkSel = false;
    _cb->SetSelection(_cb->GetCount() - 1);
    _bookmarkSel = true;
}

void CHMFrame::OnRemoveBookmark(wxCommandEvent&)
{
    if (!_cb->GetCount())
        return;

    _cb->Delete(_cb->GetSelection());
    _bookmarksDeleted = true;

    if (_cb->GetCount()) {
        _bookmarkSel = false;
        _cb->SetSelection(0);
        _bookmarkSel = true;
    } else
        _cb->SetValue(wxEmptyString);
}

void CHMFrame::OnBookmarkSel(wxCommandEvent& event)
{
    if (!_bookmarkSel)
        return;

    wxString* url = reinterpret_cast<wxString*>(
#ifdef __WXGTK__
        _cb->GetClientData(event.GetSelection()));
#else
        _cb->wxItemContainer::GetClientData(event.GetSelection()));
#endif

    if (!url || url->IsEmpty())
        return;

    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    _nbhtml->LoadPageInCurrentView(wxT("file:") + chmf->ArchiveName() + wxT("#xchm:/") + *url);
}

void CHMFrame::OnSelectionChanged(wxTreeEvent& event)
{
    auto id   = event.GetItem();
    auto chmf = CHMInputStream::GetCache();

    if (id == _tcl->GetRootItem() || !chmf || !id.IsOk())
        return;

    auto data = dynamic_cast<URLTreeItem*>(_tcl->GetItemData(id));

    if (!data || data->_url.IsEmpty())
        return;

    if (!_nbhtml->GetCurrentPage()->IsCaller()) {
        _nbhtml->GetCurrentPage()->SetSync(false);
        _nbhtml->LoadPageInCurrentView(wxT("file:") + chmf->ArchiveName() + wxT("#xchm:/") + data->_url);
        _nbhtml->GetCurrentPage()->SetSync(true);
    }
}

void CHMFrame::OnCloseWindow(wxCloseEvent&)
{
    SaveExitInfo();
    SaveBookmarks();
    Destroy();
}

void CHMFrame::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_F9) {
        wxCommandEvent dummy;
        OnShowContents(dummy);
    }

    event.Skip();
}

bool CHMFrame::LoadCHM(const wxString& archive)
{
    wxBusyCursor bc;
    wxLogNull    wln;

    auto rtn = false;

    SaveBookmarks();

    _nb->SetSelection(0);
    _nbhtml->CloseAllPagesExceptFirst();

    if (!archive.StartsWith(wxT("file:")) || !archive.Contains(wxT("#xchm:"))) {
        wxFileSystem              wfs;
        std::unique_ptr<wxFSFile> p(wfs.OpenFile(wxT("file:") + archive + wxT("#xchm:/")));

        auto chmf = CHMInputStream::GetCache();

        if (!chmf)
            return false;

        rtn = _nbhtml->LoadPageInCurrentView(wxT("file:") + chmf->ArchiveName() + wxT("#xchm:") + chmf->HomePage());
    } else
        rtn = _nbhtml->LoadPageInCurrentView(archive);

    if (!rtn) { // Error, could not load CHM file
        if (_tcl->GetCount()) {
            _tcl->Unselect();
            _tcl->DeleteChildren(_tcl->GetRootItem());
        }

        if (_sw->IsSplit()) {
            _sw->Unsplit(_nb);
            _nb->Show(false);
        }

        _menuFile->Check(ID_Contents, false);
        _tb->ToggleTool(ID_Contents, false);
        _cip->Reset();
        _csp->Reset();
        _nbhtml->LoadPageInCurrentView(wxT("memory:error.html"));

    } else {
        UpdateCHMInfo();
        LoadBookmarks();
    }

    return rtn;
}

bool CHMFrame::LoadContextID(int contextID)
{
    wxBusyCursor bc;

    auto chmf = CHMInputStream::GetCache();

    if (!chmf || !chmf->IsValidCID(contextID))
        return false;

    return _nbhtml->LoadPageInCurrentView(wxT("file:") + chmf->ArchiveName() + wxT("#xchm:")
                                          + chmf->GetPageByCID(contextID));
}

void CHMFrame::UpdateCHMInfo()
{
#if !wxUSE_UNICODE
    static auto noSpecialFont = true;
    static auto enc           = wxFont::GetDefaultEncoding();
#endif
    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    wxWindowDisabler wwd;
    wxBusyInfo       wait(_("Loading, please wait.."), this);

    auto filename = chmf->ArchiveName();

    if (!filename.IsEmpty()) {
        _fh.AddFileToHistory(filename);

        if (!_menuFile->IsEnabled(ID_Recent))
            _menuFile->Enable(ID_Recent, true);
    }

    _nbhtml->GetCurrentPage()->HistoryClear();
    _csp->Reset();
    _cip->Reset();

    auto title = chmf->Title();

    if (_tcl->GetCount()) {
        _tcl->Unselect();
        _tcl->DeleteChildren(_tcl->GetRootItem());
    }

#if !wxUSE_UNICODE
    auto fontFace = chmf->DefaultFont();

    if (!fontFace.IsEmpty()) {
        auto fs = -1L;

        fontFace.BeforeLast(wxT(',')).AfterLast(wxT(',')).ToLong(&fs);

        fs = std::max(fs, 10L);

        wxFont font(static_cast<int>(fs), wxDEFAULT, wxNORMAL, wxNORMAL, false, fontFace.BeforeFirst(wxT(',')),
                    chmf->DesiredEncoding());

        if (font.Ok()) {
            _tcl->SetFont(font);
            _csp->SetNewFont(font);
            _cip->SetNewFont(font);
            _cb->SetFont(font);

            auto sizes = ComputeFontSizes(_fontSize);
            _nbhtml->SetChildrenFonts(font.GetFaceName(), font.GetFaceName(), sizes.data());

            noSpecialFont = false;
        }

    } else if (!noSpecialFont) {
        _tcl->SetFont(_font);

        wxFont tmp(_font.GetPointSize(), _font.GetFamily(), _font.GetStyle(), _font.GetWeight(), _font.GetUnderlined(),
                   _font.GetFaceName(), enc);

        if (tmp.Ok()) {
            _cb->SetFont(tmp);
            _csp->SetNewFont(tmp);
            _cip->SetNewFont(tmp);
        }

        auto sizes = ComputeFontSizes(_fontSize);
        _nbhtml->SetChildrenFonts(_normalFont, _fixedFont, sizes.data());

        noSpecialFont = true;
    }
#endif
    if (_loadTopics)
        chmf->GetTopicsTree(*_tcl);

    if (_loadIndex)
        chmf->GetIndex(*_cip->GetResultsList());

    if (!title.IsEmpty())
        SetTitle(wxT("xCHM v. " VERSION ": ") + title);
    else
        SetTitle(wxT("xCHM v. " VERSION));

    // if we have contents..
    if (_tcl->GetCount() >= 1) {
        if (!_sw->IsSplit()) {
            _nb->Show(true);
            _sw->SplitVertically(_nb, _nbhtml, _sashPos);
            _menuFile->Check(ID_Contents, true);
            _tb->ToggleTool(ID_Contents, true);
        }

    } else {
        if (_sw->IsSplit()) {
            _sw->Unsplit(_nb);
            _nb->Show(false);
        }

        _menuFile->Check(ID_Contents, false);
        _tb->ToggleTool(ID_Contents, false);
    }

    // select Contents
    _nb->SetSelection(0);
}

wxMenuBar* CHMFrame::CreateMenu()
{
    _menuFile = new wxMenu;

    _menuFile->Append(ID_Open, _("&Open..\tCtrl-O"), OPEN_HELP);
    _menuFile->Append(ID_Print, _("&Print page..\tCtrl-P"), PRINT_HELP);
    _menuFile->Append(ID_Fonts, _("Fon&ts.."), FONTS_HELP);
    _menuFile->AppendSeparator();
    _menuFile->AppendCheckItem(ID_Contents, _("&Show contents tree\tCtrl-S"), CONTENTS_HELP);
    _menuFile->AppendSeparator();

#if defined(__WXMSW__) || defined(__WXMAC__)
    _menuFile->Append(ID_RegisterExtension, _("&Make xCHM the default CHM viewer"), REGISTER_EXTENSION_HELP);
    _menuFile->AppendSeparator();
#endif

    auto recent = new wxMenu;
    _menuFile->Append(ID_Recent, _("&Recent files"), recent);
    _fh.UseMenu(recent);

    // Fill the file history menu.
    wxConfig config(wxT("xchm"));
    config.SetPath(wxT("/Recent"));
    _fh.Load(config);

    if (_fh.GetCount() == 0)
        _menuFile->Enable(ID_Recent, false);

    _menuFile->AppendSeparator();
    _menuFile->Append(ID_Quit, _("E&xit\tCtrl-X"), _("Quit the application."));

    auto menuHistory = new wxMenu;

    menuHistory->Append(ID_Home, _("&Home\tCtrl-H"), HOME_HELP);
    menuHistory->Append(ID_Forward, _("For&ward\tAlt-RIGHT"), FORWARD_HELP);
    menuHistory->Append(ID_Back, _("&Back\tAlt-LEFT"), BACK_HELP);

    auto menuHelp = new wxMenu;
    menuHelp->Append(ID_About, _("&About..\tF1"), ABOUT_HELP);

    auto menuEdit = new wxMenu;
    menuEdit->Append(ID_CopySelection, _("&Copy\tCtrl-C"), COPY_HELP);
    menuEdit->Append(ID_FindInPage, _("&Find..\tCtrl-F"), FIND_HELP);
    menuEdit->AppendSeparator();
    menuEdit->Append(ID_CloseTab, _("&Close tab\tCtrl-W"), CLOSETAB_HELP);
    menuEdit->Append(ID_NewTab, _("&New tab\tCtrl-T"), NEWTAB_HELP);

    auto menuView = new wxMenu;
    menuView->Append(ID_FullScreen, _("Toggle &fullscreen\tF11"), FULLSCREEN_HELP);

    auto menuBar = new wxMenuBar;
    menuBar->Append(_menuFile, _("&File"));
    menuBar->Append(menuView, _("&View"));
    menuBar->Append(menuEdit, _("&Edit"));
    menuBar->Append(menuHistory, _("Hi&story"));
    menuBar->Append(menuHelp, _("&Help"));

    return menuBar;
}

namespace {

#include <hbook_closed.xpm>
#include <hbook_open.xpm>
#include <hpage.xpm>

} // namespace

wxPanel* CHMFrame::CreateContentsPanel()
{
    auto temp   = new wxPanel(_nb);
    auto sizer  = new wxBoxSizer(wxVERTICAL);
    auto bmarks = new wxStaticBoxSizer(new wxStaticBox(temp, -1, _("Bookmarks")), wxVERTICAL);
    auto inner  = new wxBoxSizer(wxHORIZONTAL);

    temp->SetAutoLayout(true);
    temp->SetSizer(sizer);

    auto il = new wxImageList(16, 16);
    il->Add(wxIcon(hbook_closed_xpm));
    il->Add(wxIcon(hbook_open_xpm));
    il->Add(wxIcon(hpage_xpm));

    _tcl = new wxTreeCtrl(temp, ID_TreeCtrl, wxDefaultPosition, wxDefaultSize,
                          wxSUNKEN_BORDER | wxTR_HIDE_ROOT | wxTR_LINES_AT_ROOT);

    _tcl->AssignImageList(il);
    _tcl->AddRoot(_("Topics"));

    _cb = new wxComboBox(temp, ID_Bookmarks, wxT(""), wxDefaultPosition, wxDefaultSize, 0, nullptr,
                         wxCB_DROPDOWN | wxCB_READONLY);
    sizer->Add(_tcl, 1, wxEXPAND, 0);
    sizer->Add(bmarks, 0, wxEXPAND | wxALL, 0);

    bmarks->Add(_cb, 0, wxEXPAND | wxBOTTOM, 5);
    bmarks->Add(inner, 1, wxEXPAND, 0);

    auto badd    = new wxButton(temp, ID_Add, _("Add"));
    auto bremove = new wxButton(temp, ID_Remove, _("Remove"));

#if wxUSE_TOOLTIPS
    badd->SetToolTip(_("Add displayed page to bookmarks."));
    bremove->SetToolTip(_("Remove selected bookmark."));
#endif

    inner->Add(badd, 1, wxEXPAND | wxRIGHT, 2);
    inner->Add(bremove, 1, wxEXPAND | wxLEFT, 2);

    return temp;
}

void CHMFrame::LoadBookmarks()
{
    _cb->Clear();
    _cb->SetValue(wxEmptyString);

    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    wxConfig config(wxT("xchm"));
    auto     bookname = chmf->ArchiveName();
    bookname.Replace(wxT("/"), wxT("."), true);

    bookname = wxT("/Bookmarks/") + bookname;
    long noEntries;

    config.SetPath(bookname);
    wxString title, url;

    if (config.Read(wxT("noEntries"), &noEntries)) {
        auto format1 = wxT("bookmark_%ld_title");
        auto format2 = wxT("bookmark_%ld_url");

        for (auto i = 0L; i < noEntries; ++i) {
            config.Read(wxString::Format(format1, i), &title);
            config.Read(wxString::Format(format2, i), &url);

            _cb->Append(title, new wxString(url));
        }
    }
}

void CHMFrame::SaveBookmarks()
{
    auto noEntries = _cb->GetCount();

    if (!_bookmarksDeleted && noEntries == 0)
        return;

    auto chmf = CHMInputStream::GetCache();

    if (!chmf)
        return;

    wxConfig config(wxT("xchm"));
    auto     bookname = chmf->ArchiveName();
    bookname.Replace(wxT("/"), wxT("."), true);
    bookname = wxT("/Bookmarks/") + bookname;

    if (_bookmarksDeleted)
        config.DeleteGroup(bookname);

    if (noEntries == 0)
        return;

    config.SetPath(bookname);
    config.Write(wxT("noEntries"), noEntries);

    auto format1 = wxT("bookmark_%ld_title");
    auto format2 = wxT("bookmark_%ld_url");

    for (decltype(noEntries) i = 0; i < noEntries; ++i) {
        auto url = reinterpret_cast<wxString*>(
#ifdef __WXGTK__
            _cb->GetClientData(i));
#else
            _cb->wxItemContainer::GetClientData(i));
#endif

        config.Write(wxString::Format(format1, i), _cb->GetString(i));

        if (url)
            config.Write(wxString::Format(format2, i), *url);
    }
}

void CHMFrame::SaveExitInfo()
{
    int  xorig, yorig, width, height;
    auto sashPos = _sw->IsSplit() ? _sw->GetSashPosition() : _sashPos;

    GetPosition(&xorig, &yorig);
    GetSize(&width, &height);

    wxConfig config(wxT("xchm"));

    config.Write(wxT("/Position/xOrig"), xorig);
    config.Write(wxT("/Position/yOrig"), yorig);
    config.Write(wxT("/Position/width"), width);
    config.Write(wxT("/Position/height"), height);
    config.Write(wxT("/Paths/lastOpenedDir"), _openPath);
    config.Write(wxT("/Fonts/normalFontFace"), _normalFont);
    config.Write(wxT("/Fonts/fixedFontFace"), _fixedFont);
    config.Write(wxT("/Fonts/size"), _nbhtml->FontSize());
    config.Write(wxT("/Sash/leftMargin"), sashPos);

    config.SetPath(wxT("/Recent"));
    _fh.Save(config);
}

namespace {

#include <back.xpm>
#include <copy.xpm>
#include <fileopen.xpm>
#include <find.xpm>
#include <forward.xpm>
#include <fullscreen.xpm>
#include <helpicon.xpm>
#include <home.xpm>
#include <htmoptns.xpm>
#include <htmsidep.xpm>
#include <print.xpm>

} // namespace

bool CHMFrame::InitToolBar(wxToolBar* toolbar)
{
    toolbar->AddTool(ID_Open, _("Open .."), wxBitmap(fileopen_xpm), OPEN_HELP);
    toolbar->AddTool(ID_Print, _("Print .."), wxBitmap(print_xpm), PRINT_HELP);
    toolbar->AddTool(ID_Fonts, _("Fonts .."), wxBitmap(htmoptns_xpm), FONTS_HELP);
    toolbar->AddCheckTool(ID_Contents, _("Contents"), wxBitmap(htmsidep_xpm), wxBitmap(htmsidep_xpm), CONTENTS_HELP);

    toolbar->AddSeparator();
    toolbar->AddTool(ID_CopySelection, _("Copy"), wxBitmap(copy_xpm), COPY_HELP);
    toolbar->AddTool(ID_FindInPage, _("Find"), wxBitmap(find_xpm), FIND_HELP);

    toolbar->AddSeparator();

    toolbar->AddTool(ID_FullScreen, _("Fullscreen"), wxBitmap(fullscreen_xpm), FULLSCREEN_HELP);

    toolbar->AddSeparator();

    toolbar->AddTool(ID_Back, _("Back"), wxBitmap(back_xpm), BACK_HELP);
    toolbar->AddTool(ID_Forward, _("Forward"), wxBitmap(forward_xpm), FORWARD_HELP);
    toolbar->AddTool(ID_Home, _("Home"), wxBitmap(home_xpm), HOME_HELP);
    toolbar->AddSeparator();
    toolbar->AddTool(ID_About, _("About"), wxBitmap(helpicon_xpm), ABOUT_HELP);

    toolbar->Realize();

    return true;
}

void CHMFrame::AddHtmlView(const wxString& path, const wxString& link)
{
    _nbhtml->AddHtmlView(path, link);
}

void CHMFrame::ToggleFullScreen(bool onlyIfFullScreenOn)
{
    if (onlyIfFullScreenOn && !_fullScreen)
        return;

    wxCommandEvent dummy;
    OnToggleFullScreen(dummy);
}

FontSizesArray CHMFrame::ComputeFontSizes(int size) const
{
    FontSizesArray sizes;

    for (auto i = -3; i <= 3; ++i)
        sizes[i + 3] = size + i * 2;

    return sizes;
}

BEGIN_EVENT_TABLE(CHMFrame, wxFrame)
EVT_MENU(ID_Quit, CHMFrame::OnQuit)
EVT_MENU(ID_About, CHMFrame::OnAbout)
EVT_MENU(ID_Open, CHMFrame::OnOpen)
EVT_MENU(ID_Fonts, CHMFrame::OnChangeFonts)
EVT_MENU(ID_Home, CHMFrame::OnHome)
EVT_MENU(ID_Forward, CHMFrame::OnHistoryForward)
EVT_MENU(ID_Back, CHMFrame::OnHistoryBack)
EVT_MENU(ID_Contents, CHMFrame::OnShowContents)
#if defined(__WXMSW__) || defined(__WXMAC__)
EVT_MENU(ID_RegisterExtension, CHMFrame::OnRegisterExtension)
#endif
EVT_MENU(ID_Print, CHMFrame::OnPrint)
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, CHMFrame::OnHistFile)
EVT_MENU(ID_FindInPage, CHMFrame::OnFind)
EVT_MENU(ID_CloseTab, CHMFrame::OnCloseTab)
EVT_MENU(ID_NewTab, CHMFrame::OnNewTab)
EVT_MENU(ID_CopySelection, CHMFrame::OnCopySelection)
EVT_MENU(ID_FullScreen, CHMFrame::OnToggleFullScreen)
EVT_BUTTON(ID_Add, CHMFrame::OnAddBookmark)
EVT_BUTTON(ID_Remove, CHMFrame::OnRemoveBookmark)
EVT_TREE_SEL_CHANGED(ID_TreeCtrl, CHMFrame::OnSelectionChanged)
EVT_COMBOBOX(ID_Bookmarks, CHMFrame::OnBookmarkSel)
EVT_TEXT_ENTER(ID_Bookmarks, CHMFrame::OnBookmarkSel)
EVT_CLOSE(CHMFrame::OnCloseWindow)
EVT_CHAR(CHMFrame::OnChar)
END_EVENT_TABLE()
