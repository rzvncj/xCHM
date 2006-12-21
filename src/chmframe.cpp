/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#include <chmframe.h>
#include <chminputstream.h>
#include <chmhtmlwindow.h>
#include <chmfontdialog.h>
#include <chmsearchpanel.h>
#include <chmindexpanel.h>
#include <chmlistctrl.h>
#include <hhcparser.h>
#include <wx/fontenum.h>
#include <wx/statbox.h>
#include <wx/accel.h>
#include <wx/filesys.h>
#include <wx/mimetype.h>
#include <wx/imaglist.h>
#include <wx/bitmap.h>
#include <wx/fs_mem.h>

#include <wx/busyinfo.h>
#include <wx/progdlg.h>

#define OPEN_HELP _("Open a CHM book.")
#define FONTS_HELP _("Change fonts.")
#define PRINT_HELP _("Print the page currently displayed.")
#define CONTENTS_HELP _("On or off?")
#define HOME_HELP _("Go to the book's start page.")
#define FORWARD_HELP _("Go forward in history. Per book.")
#define BACK_HELP _("Back to the last visited page. Per book.")
#define ABOUT_HELP _("About the program.")
#define COPY_HELP _("Copy selection.")
#define FIND_HELP _("Find word in page.")
#define REGISTER_EXTENSION_HELP _("Associate the .chm file " \
				  "extension with xCHM.")


namespace {

const wxChar *greeting = wxT(
	"<html><body><table border=0><tr><td align=\"left\">"
	"<img src=\"memory:logo.xpm\"></td><td align=\"left\">"
	"Hello, and welcome to <B>xCHM</B>, the UNIX CHM viewer."
	"<br><br><B>xCHM</B> has been written by Razvan Cojocaru "
	"(razvanco@gmx.net). It is licensed under the <TT>GPL</TT>.<br>"
	"<B>xCHM</B> is based on Jed Wing's <a href=\"http://66.93.236.84/"
	"~jedwin/projects/chmlib/\">CHMLIB</a> and <a href=\"http://www."
	"wxwidgets.org\">wxWidgets</a>.</td></tr></table>"
	"<br>If you'd like to know more"
	" about CHM, you could check out <a href=\"http://www.speakeasy."
	"org/~russotto/chm/\">Matthew Russoto's CHM page</a> or <a"
	" href=\"http://www.nongnu.org/chmspec/latest/\">Pabs' CHM"
	" Specification page</a>.<br>Pabs has contributed time and knowledge"
	" to the development of <B>xCHM</B>, and features such as the fast"
	" index search would not have been implemented without his help."
	" Portions of the fast index search are modified versions of"
	" Pabs' <TT>GPL</TT>d <TT>chmdeco</TT> code.<br><br>If"
	" you'd like to use the code in your own stuff please figure"
	" <TT>GPL</TT> out first. Far too many people think <TT>GPL</TT> is"
	" bad out of utter ignorance.<br><br>Tips:<br><ul><li>"
	"The global search is an \'AND\' search, i.e. searching for"
	" \'word1 word2\' (quotes not included) will find all the pages"
	" that contain both word1 and word2, in whatever order.</li>"
	"<li>Allowing partial matches will find pages that contain"
	" words that only start with the typed strings, i.e. searching"
	" for \'ans 4\' (quotes not included) will find pages that"
	" contain the sentence \'The answer is 42.\'.</li><li>Right"
	" clicking on the displayed page brings out a popup menu with"
	" common options.</li></ul><br><br>Ctrl(cmd)-C is copy,"
	" Ctrl(cmd)-F is find in page.<br><br>Enjoy.</body></html>");


const wxChar *error_page = wxT(
	"<html><body>Error loading CHM file!</body></html>");


const wxChar *about_txt = wxT(
	"xCHM v. " VERSION "\nby Razvan Cojocaru <razvanco@gmx.net>\n\n"
	"With thanks to Pabs (http://bonedaddy.net/pabs3/hhm/).\n"
	"Based on Jed Wing's CHMLIB (http://66.93.236.84/~jedwin/projects/).\n"
	"XMLRPC code for context sensitive help contributed by\n"
	"Eamon Millman <millman@pcigeomatics.com>.\n"
	"<SPAN> tag support and contents tree icons contributed by\n"
	"Fritz Elfert <felfert@users.sourceforge.net>.\n"
	"Written with wxWidgets (http://www.wxwidgets.org).\n\n"
	"This program is (proudly) under the GPL.");

#include <xchm-32.xpm>
#include <logo.xpm>

} // namespace


CHMFrame::CHMFrame(const wxString& title, const wxString& booksDir, 
		   const wxPoint& pos, const wxSize& size,
		   const wxString& normalFont, const wxString& fixedFont,
		   const int fontSize, const int sashPosition,
		   const wxString& fullAppPath)
	: wxFrame(NULL, -1, title, pos, size), _html(NULL),
	  _tcl(NULL), _sw(NULL), _menuFile(NULL), _tb(NULL), _ep(NULL),
	  _nb(NULL), _cb(NULL), _csp(NULL), _cip(NULL), _openPath(booksDir), 
	  _normalFonts(NULL), _fixedFonts(NULL), _normalFont(normalFont), 
	  _fixedFont(fixedFont), _fontSize(fontSize), _bookmarkSel(true),
	  _bookmarksDeleted(false), _sashPos(sashPosition),
	  _fullAppPath(fullAppPath)
{
#	if wxUSE_ACCEL
	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, (int) 'F', ID_FindInPage);
	entries[1].Set(wxACCEL_CTRL, (int) 'C', ID_CopySelection);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
#	endif

#ifdef __WXMAC__	
	wxApp::s_macAboutMenuItemId = ID_About;
#endif

	int sizes[7];
	for(int i = -3; i <= 3; ++i)
		sizes[i+3] = _fontSize + i * 2;

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
	wxMemoryFSHandler::AddFile(wxT("logo.xpm"), bitmap, //wxBITMAP(logo), 
				   wxBITMAP_TYPE_XPM);
	wxMemoryFSHandler::AddFile(wxT("about.html"), greeting);
	wxMemoryFSHandler::AddFile(wxT("error.html"), error_page);

	_ep = new wxHtmlEasyPrinting(wxT("Printing"), this);

	_sw = new wxSplitterWindow(this);
	_sw->SetMinimumPaneSize(CONTENTS_MARGIN);

	_nb = new wxNotebook(_sw, ID_Notebook);
	_nb->Show(FALSE);

	wxPanel* temp = CreateContentsPanel();

	_html = new CHMHtmlWindow(_sw, _tcl, this);
	_html->SetRelatedFrame(this, wxT("xCHM v. " VERSION));
	_html->SetRelatedStatusBar(0);

	_html->SetFonts(_normalFont, _fixedFont, sizes);	
	_html->LoadPage(wxT("memory:about.html"));

	_csp = new CHMSearchPanel(_nb, _tcl, _html);
	_font = _tcl->GetFont();

	_cip = new CHMIndexPanel(_nb, _html);

	_nb->AddPage(temp, _("Contents"));
	_nb->AddPage(_cip, _("Index"));
	_nb->AddPage(_csp, _("Search"));

	_sw->Initialize(_html);
	_html->SetFocusFromKbd();
}


CHMFrame::~CHMFrame()
{
	delete _ep;
	delete _fixedFonts;
	delete _normalFonts;
}


void CHMFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}


void CHMFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	::wxMessageBox(about_txt, _("About xCHM"),
		       wxOK | wxICON_INFORMATION, this );
}


void CHMFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	wxString selection =
		::wxFileSelector(_("Choose a file.."), _openPath, 
				 wxEmptyString, wxT("chm"),
#ifndef __WXMOTIF__
				 // they say Motif can't handle the following.
				 wxT("CHM files (*.chm)|*.chm;*.CHM|"
				 "All files (*.*)|*.*"), 
#else
				 wxT("All files (*.*)|*.*"),
#endif
				 wxOPEN | wxFILE_MUST_EXIST, this);

	if(selection.IsEmpty() || !_tcl)
		return;

	_openPath = selection.BeforeLast(wxT('/'));
	LoadCHM(selection);
}


void CHMFrame::OnChangeFonts(wxCommandEvent& WXUNUSED(event))
{
	// First time initialization only.
	if(_normalFonts == NULL) {
		wxFontEnumerator enu;
		enu.EnumerateFacenames();
		_normalFonts = new wxArrayString;

#if wxMAJOR_VERSION == 2 && wxMINOR_VERSION >= 7
		*_normalFonts = enu.GetFacenames();
#else
		*_normalFonts = *enu.GetFacenames();
#endif
		_normalFonts->Sort();
	}

	if(_fixedFonts == NULL) {
		wxFontEnumerator enu;
		enu.EnumerateFacenames(wxFONTENCODING_SYSTEM, TRUE);
		_fixedFonts = new wxArrayString;

#if wxMAJOR_VERSION == 2 && wxMINOR_VERSION >= 7
		*_fixedFonts = enu.GetFacenames();
#else
		*_fixedFonts = *enu.GetFacenames();
#endif
		_fixedFonts->Sort();
	}


	assert(_normalFonts != NULL);
	assert(_fixedFonts != NULL);

	CHMFontDialog cfd(this, _normalFonts, _fixedFonts, _normalFont,
			  _fixedFont, _fontSize);

	if(cfd.ShowModal() == wxID_OK) {
		
		wxBusyCursor bc;

		_html->SetFonts(_normalFont = cfd.NormalFont(), 
				_fixedFont = cfd.FixedFont(),
				cfd.Sizes());

		_fontSize = *(cfd.Sizes() + 3);

		wxString page = _html->GetOpenedPage();

		if(page.IsEmpty())
			_html->LoadPage(wxT("memory:about.html"));
	}
}


void CHMFrame::OnHome(wxCommandEvent& WXUNUSED(event))
{
	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	_html->LoadPage(wxString(wxT("file:")) + chmf->ArchiveName()
		+ wxT("#xchm:") + chmf->HomePage());
}


void CHMFrame::OnHistoryForward(wxCommandEvent& WXUNUSED(event))
{
	_html->HistoryForward();
}


void CHMFrame::OnHistoryBack(wxCommandEvent& WXUNUSED(event))
{
	if(_html->HistoryCanBack())
		_html->HistoryBack();
	else {
		if(CHMInputStream::GetCache())
			return;
	}
}


void CHMFrame::OnShowContents(wxCommandEvent& WXUNUSED(event))
{
	if(_sw->IsSplit()) {
		_tb->ToggleTool(ID_Contents, FALSE);
		_menuFile->Check(ID_Contents, FALSE);
		_sashPos = _sw->GetSashPosition();
		
		_sw->Unsplit(_nb);
		_nb->Show(FALSE);
	} else {		
			
		//if(_tcl->GetCount() >= 1) {
			
			_tb->ToggleTool(ID_Contents, TRUE);
			_menuFile->Check(ID_Contents, TRUE);

			_nb->Show(TRUE);
			_sw->SplitVertically(_nb, _html, _sashPos);

		/*} else {
			_tb->ToggleTool(ID_Contents, FALSE);
			_menuFile->Check(ID_Contents, FALSE);
			
			::wxMessageBox(_("Couldn't extract the book"
				       " contents tree."), 
				       _("No contents.."), 
				       wxOK | wxICON_WARNING, this );
	       }*/
	}
}


#if defined(__WXMSW__) || defined(__WXMAC__)
void CHMFrame::OnRegisterExtension(wxCommandEvent& WXUNUSED(event))
{
	int answer = wxMessageBox(_("This is experimental code, and "
				    "doing this will overwrite any "
				    "previously registered CHM viewer "
				    "associations.\n\nAre you sure "
				    "you know what you're doing?"),
				  _("Confirm"),
				  wxOK | wxCANCEL, this);

	if(answer == wxOK) {
		wxFileTypeInfo fti(wxT("application/x-chm"), _fullAppPath,
				   wxEmptyString, wxT("Compiled HTML help"),
				   wxT("chm"), NULL);

		wxFileType *ft = wxTheMimeTypesManager->Associate(fti);

		if(ft) {
			ft->SetDefaultIcon(_fullAppPath);
			wxMessageBox(_("Registration successful!"), 
				     _("Done"), wxOK, this);
		}
	}
}
#endif// __WXMSW__


void CHMFrame::OnPrint(wxCommandEvent& WXUNUSED(event))
{
        int sizes[7];
	        for(int i = -3; i <= 3; ++i)
	                sizes[i+3] = _fontSize + i * 2;
			
	_ep->SetFonts(_normalFont, _fixedFont, sizes);
	_ep->PrintFile(_html->GetOpenedPage());
}


void CHMFrame::OnHistFile(wxCommandEvent& event)
{
      wxString f(_fh.GetHistoryFile(event.GetId() - wxID_FILE1));
      if (!f.IsEmpty())
	      LoadCHM(f);
}


void CHMFrame::OnFind(wxCommandEvent& event)
{
	_html->OnFind(event);
}


void CHMFrame::OnCopySelection(wxCommandEvent& event)
{
	_html->OnCopy(event);
}


void CHMFrame::OnAddBookmark(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId id = _tcl->GetSelection();

	if(!id.IsOk())
		return;

	wxString title = _tcl->GetItemText(id);

 	if(title.IsEmpty())
		return;
       
	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(id));

	if(!data || (data->_url).IsEmpty())
		return;

	_cb->Append(title, new wxString(data->_url));

	_bookmarkSel = false;
	_cb->SetSelection(_cb->GetCount() - 1);
	_bookmarkSel = true;		
}


void CHMFrame::OnRemoveBookmark(wxCommandEvent& WXUNUSED(event))
{
	if(!_cb->GetCount())
		return;

	_cb->Delete(_cb->GetSelection());
	_bookmarksDeleted = true;

	if(_cb->GetCount()) {
		_bookmarkSel = false;
		_cb->SetSelection(0);
		_bookmarkSel = true;
	} else {
		_cb->SetValue(wxEmptyString);
	}
}


void CHMFrame::OnBookmarkSel(wxCommandEvent& event)
{
	if(!_bookmarkSel)
		return;

	wxString *url = reinterpret_cast<wxString *>(
#ifdef __WXGTK__
		_cb->GetClientData(event.GetSelection()));
#else
		_cb->wxItemContainer::GetClientData(event.GetSelection()));
#endif

	if(!url || url->IsEmpty())
		return;

	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	_html->LoadPage(wxString(wxT("file:")) + chmf->ArchiveName() +
			wxT("#xchm:/") + *url);
}


void CHMFrame::OnSelectionChanged(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	CHMFile *chmf = CHMInputStream::GetCache();

	if(id == _tcl->GetRootItem() || !chmf)
		return;

	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(id));

	if(!data || data->_url.IsEmpty())
		return;

	if(!_html->IsCaller()) {
		_html->SetSync(false);
		_html->LoadPage(wxString(wxT("file:")) + chmf->ArchiveName() +
				wxT("#xchm:/") + data->_url);
		_html->SetSync(true);
	}
}


void CHMFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
	SaveExitInfo();
	SaveBookmarks();
	Destroy();
}


void CHMFrame::OnChar(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_F9) {
		wxCommandEvent dummy;
		OnShowContents(dummy);
	}

	event.Skip();
}


bool CHMFrame::LoadCHM(const wxString& archive)
{
	wxBusyCursor bc;
	wxLogNull wln;
	
	bool rtn = false;
	SaveBookmarks();
	_nb->SetSelection(0);

	if(!archive.StartsWith(wxT("file:")) || 
	   !archive.Contains(wxT("#xchm:"))) {

		wxFileSystem wfs;
		wxFSFile *f = wfs.OpenFile(wxString(wxT("file:")) + archive +
			      wxT("#xchm:/"));
		delete f;
	
	        CHMFile *chmf = CHMInputStream::GetCache();

		if(!chmf)
			return false;
	
		rtn = _html->LoadPage(wxString(wxT("file:"))
						+ chmf->ArchiveName()
				                + wxT("#xchm:") 
						+ chmf->HomePage());
	} else {
		rtn = _html->LoadPage(archive);
	}

	if(!rtn) { // Error, could not load CHM file
		if(_tcl->GetCount())
			_tcl->CollapseAndReset(_tcl->GetRootItem());
		if(_sw->IsSplit()) {
			_sw->Unsplit(_nb);
			_nb->Show(FALSE);
		}
		_menuFile->Check(ID_Contents, FALSE);
		_tb->ToggleTool(ID_Contents, FALSE);
		_cip->Reset();
		_csp->Reset();
		_html->LoadPage(wxT("memory:error.html"));

	} else {
		UpdateCHMInfo();
		LoadBookmarks();
	}

	return rtn;
}


bool CHMFrame::LoadContextID( const int contextID )
{
	wxBusyCursor bc;

	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return FALSE;

	if( !chmf->IsValidCID( contextID ) )
		return FALSE;

	return _html->LoadPage(wxString(wxT("file:")) + chmf->ArchiveName()
		+ wxT("#xchm:") + chmf->GetPageByCID( contextID ) );
}


void CHMFrame::UpdateCHMInfo()
{
#if !wxUSE_UNICODE
	static bool noSpecialFont = true;
	static wxFontEncoding enc = wxFont::GetDefaultEncoding();
#endif
	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	wxString filename = chmf->ArchiveName();
	if(!filename.IsEmpty()) {		
		_fh.AddFileToHistory(filename);

		if(!_menuFile->IsEnabled(ID_Recent))
			_menuFile->Enable(ID_Recent, TRUE);
	}

	_html->HistoryClear();
	_csp->Reset();
	_cip->Reset();

	wxString title = chmf->Title();

	if(_tcl->GetCount())
		_tcl->CollapseAndReset(_tcl->GetRootItem());

	wxProgressDialog wpd(_("Processing.."), 
			     _("Retrieving data.."),
			     100, this, wxPD_APP_MODAL 
			     | wxPD_AUTO_HIDE | wxPD_SMOOTH 
			     | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME);

	chmf->GetTopicsTree(_tcl, &wpd);
	chmf->GetIndex(_cip->GetResultsList(), &wpd);
	wpd.Update(100, _("Done."));
	
	if(!title.IsEmpty()) {
		wxString titleBarText = 
			wxString(wxT("xCHM v. " VERSION ": ")) + title;

		SetTitle(titleBarText);
		_html->SetRelatedFrame(this, titleBarText);
	} else {
		SetTitle(wxT("xCHM v. " VERSION));
		_html->SetRelatedFrame(this, wxT("xCHM v. " VERSION));
	}

#if !wxUSE_UNICODE
	wxString fontFace = chmf->DefaultFont();

	if(!fontFace.IsEmpty()) {

		long fs = -1;		
		fontFace.BeforeLast(wxT(',')).AfterLast(wxT(',')).ToLong(&fs);

		wxFont font((int)fs, wxDEFAULT, wxNORMAL, wxNORMAL, 
			    FALSE, fontFace.BeforeFirst(wxT(',')), 
			    chmf->DesiredEncoding());

		if(font.Ok()) {

			int sizes[7];
			for(int i = -3; i <= 3; ++i)
				sizes[i+3] = _fontSize + i * 2;

			_tcl->SetFont(font);
			_csp->SetNewFont(font);
			_cip->SetNewFont(font);
			_cb->SetFont(font);
			_html->SetFonts(font.GetFaceName(), font.GetFaceName(),
					sizes);
			
			noSpecialFont = false;
		}

	} else if(noSpecialFont == false) {

		int sizes[7];
		for(int i = -3; i <= 3; ++i)
			sizes[i+3] = _fontSize + i * 2;

		_tcl->SetFont(_font);

		wxFont tmp(_font.GetPointSize(), 
			   _font.GetFamily(),
			   _font.GetStyle(), _font.GetWeight(), 
			   _font.GetUnderlined(), _font.GetFaceName(),
			   enc);

		if(tmp.Ok()) {
			_cb->SetFont(tmp);
			_csp->SetNewFont(tmp);
			_cip->SetNewFont(tmp);
		}

		_html->SetFonts(_normalFont, _fixedFont, sizes);
		noSpecialFont = true;
	}
#endif
	
	// if we have contents..
	if(_tcl->GetCount() >= 1) {		
		if(!_sw->IsSplit()) {
			_nb->Show(TRUE);
			_sw->SplitVertically(_nb, _html, _sashPos);
			_menuFile->Check(ID_Contents, TRUE);
			_tb->ToggleTool(ID_Contents, TRUE);
		}
	} else {

		if(_sw->IsSplit()) {
			_sw->Unsplit(_nb);
			_nb->Show(FALSE);
		}
		_menuFile->Check(ID_Contents, FALSE);
		_tb->ToggleTool(ID_Contents, FALSE);
	}

	// select Contents
	_nb->SetSelection(0);	
}


wxMenuBar* CHMFrame::CreateMenu()
{
	_menuFile = new wxMenu;

	_menuFile->Append(ID_Open, _("&Open..\tCtrl-O"), OPEN_HELP);
	_menuFile->Append(ID_Print, _("&Print page..\tCtrl-P"), PRINT_HELP);
	_menuFile->Append(ID_Fonts, _("Fon&ts..\tCtrl-T"), FONTS_HELP);
	_menuFile->AppendSeparator();
	_menuFile->AppendCheckItem(ID_Contents, 
				   _("&Show contents tree\tCtrl-S"),
				   CONTENTS_HELP);
	_menuFile->AppendSeparator();

#if defined(__WXMSW__) || defined(__WXMAC__)
	_menuFile->Append(ID_RegisterExtension, 
			  _("&Make xCHM the default CHM viewer"),
			  REGISTER_EXTENSION_HELP);
	_menuFile->AppendSeparator();
#endif

	wxMenu *recent = new wxMenu;
	_menuFile->Append(ID_Recent, _("&Recent files"), recent);
	_fh.UseMenu(recent);

	// Fill the file history menu.
       	wxConfig config(wxT("xchm"));
	config.SetPath(wxT("/Recent"));
	_fh.Load(config);

	if(_fh.GetCount() == 0)
		_menuFile->Enable(ID_Recent, FALSE);

	_menuFile->AppendSeparator();
	_menuFile->Append(ID_Quit, _("E&xit\tCtrl-X"), 
			  _("Quit the application."));

	wxMenu *menuHistory = new wxMenu;

	menuHistory->Append(ID_Home, _("&Home\tCtrl-H"), HOME_HELP);
	menuHistory->Append(ID_Forward, _("For&ward\tCtrl-W"), FORWARD_HELP);
	menuHistory->Append(ID_Back, _("&Back\tCtrl-B"), BACK_HELP);
	
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(ID_About, _("&About..\tF1"), ABOUT_HELP);

	wxMenu *menuEdit = new wxMenu;
	menuEdit->Append(ID_CopySelection, _("&Copy\tCtrl-C"), COPY_HELP);
	menuEdit->Append(ID_FindInPage, _("&Find..\tCtrl-F"), FIND_HELP);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(_menuFile, _("&File"));
	menuBar->Append(menuEdit, _("&Edit"));
	menuBar->Append(menuHistory, _("Hi&story"));
	menuBar->Append(menuHelp, _("&Help"));


	return menuBar;
}

//#ifndef __WXMSW__
namespace {

#include <hbook_open.xpm>
#include <hbook_closed.xpm>
#include <hpage.xpm>

} // namespace
//#endif


wxPanel* CHMFrame::CreateContentsPanel()
{
	wxPanel *temp = new wxPanel(_nb);
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *bmarks = new wxStaticBoxSizer(
		new wxStaticBox(temp, -1, _("Bookmarks")), wxVERTICAL);
	wxSizer *inner = new  wxBoxSizer(wxHORIZONTAL);

	temp->SetAutoLayout(TRUE);
        temp->SetSizer(sizer);

#ifndef __WXMSW__
	wxImageList *tclBL = new wxImageList(16, 16);
	tclBL->Add(wxIcon(hbook_closed_xpm));
	tclBL->Add(wxIcon(hbook_closed_xpm));
	tclBL->Add(wxIcon(hbook_open_xpm));
	tclBL->Add(wxIcon(hbook_open_xpm));
	wxImageList *tclIL = new wxImageList(16, 16);
	tclIL->Add(wxIcon(hpage_xpm));
#else
	wxImageList *il = new wxImageList(16, 16);
	il->Add(wxIcon(hbook_closed_xpm));
	il->Add(wxIcon(hbook_open_xpm));
	il->Add(wxIcon(hpage_xpm));
#endif

	_tcl = new wxTreeCtrl(temp, ID_TreeCtrl, wxDefaultPosition, 
			      wxDefaultSize, 
#ifndef __WXMSW__
			      wxTR_HAS_BUTTONS |
#endif
			      wxSUNKEN_BORDER | wxTR_HIDE_ROOT
			      | wxTR_LINES_AT_ROOT);

#ifndef __WXMSW__
	_tcl->AssignButtonsImageList(tclBL);
	_tcl->AssignImageList(tclIL);
#else
	_tcl->AssignImageList(il);	
#endif
	_tcl->AddRoot(_("Topics"));

	_cb = new wxComboBox(temp, ID_Bookmarks, wxT(""), wxDefaultPosition,
			     wxDefaultSize, 0, NULL, wxCB_DROPDOWN 
			     | wxCB_READONLY);
	sizer->Add(_tcl, 1, wxEXPAND, 0);
	sizer->Add(bmarks, 0, wxEXPAND | wxALL, 0);

	bmarks->Add(_cb, 0, wxEXPAND | wxBOTTOM, 5);
	bmarks->Add(inner, 1, wxEXPAND, 0);

	wxButton *badd = new wxButton(temp, ID_Add, _("Add"));
	wxButton *bremove = new wxButton(temp, ID_Remove, _("Remove"));

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

	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	wxConfig config(wxT("xchm"));
	wxString bookname = chmf->ArchiveName();
	bookname.Replace(wxT("/"), wxT("."), TRUE);

	bookname = wxString(wxT("/Bookmarks/")) + bookname;
	long noEntries;

	config.SetPath(bookname);
	wxString title, url;

	if(config.Read(wxT("noEntries"), &noEntries)) {
		
		const wxChar* format1 = wxT("bookmark_%ld_title");
		const wxChar* format2 = wxT("bookmark_%ld_url");
		
		for(long i = 0; i < noEntries; ++i) {
			config.Read(wxString::Format(format1, i), &title);
			config.Read(wxString::Format(format2, i), &url);

			_cb->Append(title, new wxString(url));
		}
	}
}


void CHMFrame::SaveBookmarks()
{
	long noEntries = _cb->GetCount();

	if(!_bookmarksDeleted && noEntries == 0)
		return;

	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	wxConfig config(wxT("xchm"));
	wxString bookname = chmf->ArchiveName();
	bookname.Replace(wxT("/"), wxT("."), TRUE);
	bookname = wxString(wxT("/Bookmarks/")) + bookname;

	if(_bookmarksDeleted)
		config.DeleteGroup(bookname);

	if(noEntries == 0)
		return;

	config.SetPath(bookname);
	config.Write(wxT("noEntries"), noEntries);

	const wxChar* format1 = wxT("bookmark_%ld_title");
	const wxChar* format2 = wxT("bookmark_%ld_url");

	for(int i = 0; i < noEntries; ++i) {
		wxString *url = reinterpret_cast<wxString *>(
#ifdef __WXGTK__
			_cb->GetClientData(i));
#else
			_cb->wxItemContainer::GetClientData(i));
#endif

		config.Write(wxString::Format(format1, i), 
			     _cb->GetString(i));

		if(url)
			config.Write(wxString::Format(format2, i), *url);
	}
}


void CHMFrame::SaveExitInfo()
{
	int xorig, yorig, width, height;
	int sashPos = _sw->IsSplit() ? _sw->GetSashPosition() : _sashPos;

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
	config.Write(wxT("/Fonts/size"), _fontSize);
	config.Write(wxT("/Sash/leftMargin"), sashPos);

	config.SetPath(wxT("/Recent"));
	_fh.Save(config);
}


namespace {

#include <fileopen.xpm>
#include <print.xpm>
#include <back.xpm>
#include <forward.xpm>
#include <home.xpm>
#include <helpicon.xpm>
#include <htmsidep.xpm>
#include <htmoptns.xpm>
#include <copy.xpm>
#include <find.xpm>

} // namespace


bool CHMFrame::InitToolBar(wxToolBar *toolbar)
{
	toolbar->AddTool(ID_Open, _("Open .."), wxBitmap(fileopen_xpm),
			 OPEN_HELP);
	toolbar->AddTool(ID_Print, _("Print .."), wxBitmap(print_xpm),
			 PRINT_HELP);
	toolbar->AddTool(ID_Fonts, _("Fonts .."), wxBitmap(htmoptns_xpm),
			 FONTS_HELP);
	toolbar->AddCheckTool(ID_Contents, _("Contents"),
			      wxBitmap(htmsidep_xpm),
			      wxBitmap(htmsidep_xpm), CONTENTS_HELP);
	
	toolbar->AddSeparator();
	toolbar->AddTool(ID_CopySelection, _("Copy"), 
			wxBitmap(copy_xpm), COPY_HELP);
	toolbar->AddTool(ID_FindInPage, _("Find"), 
			wxBitmap(find_xpm), FIND_HELP);
	
	toolbar->AddSeparator();

	toolbar->AddTool(ID_Back, _("Back"), wxBitmap(back_xpm), BACK_HELP);
	toolbar->AddTool(ID_Forward, _("Forward"), wxBitmap(forward_xpm), 
			 FORWARD_HELP);
	toolbar->AddTool(ID_Home, _("Home"), wxBitmap(home_xpm), 
			 HOME_HELP);
	toolbar->AddSeparator();
	toolbar->AddTool(ID_About, _("About"), wxBitmap(helpicon_xpm), 
			 ABOUT_HELP);

	toolbar->Realize();

	return TRUE;
}


// They say this needs to be in the implementation file.
BEGIN_EVENT_TABLE(CHMFrame, wxFrame)
	EVT_MENU(ID_Quit,  CHMFrame::OnQuit)
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
	EVT_MENU(ID_CopySelection, CHMFrame::OnCopySelection)
	EVT_BUTTON(ID_Add, CHMFrame::OnAddBookmark)
	EVT_BUTTON(ID_Remove, CHMFrame::OnRemoveBookmark)
	EVT_TREE_SEL_CHANGED(ID_TreeCtrl, CHMFrame::OnSelectionChanged)
	EVT_COMBOBOX(ID_Bookmarks, CHMFrame::OnBookmarkSel)
	EVT_CLOSE(CHMFrame::OnCloseWindow)
	EVT_CHAR(CHMFrame::OnChar)
END_EVENT_TABLE()


/*
  Local Variables:
  mode: c++
  c-basic-offset: 8
  tab-width: 8
  c-indent-comments-syntactically-p: t
  c-tab-always-indent: t
  indent-tabs-mode: t
  End:
*/

// vim:shiftwidth=8:autoindent:tabstop=8:noexpandtab:softtabstop=8

