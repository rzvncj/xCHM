/*

  Copyright (C) 2003 - 2013  Razvan Cojocaru <rzvncj@gmail.com>
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


#include <chmframe.h>
#include <chminputstream.h>
#include <chmhtmlwindow.h>
#include <chmfontdialog.h>
#include <chmsearchpanel.h>
#include <chmindexpanel.h>
#include <chmlistctrl.h>
#include <chmhtmlnotebook.h>
#include <hhcparser.h>
#include <wx/fontenum.h>
#include <wx/statbox.h>
#include <wx/accel.h>
#include <wx/filesys.h>
#include <wx/mimetype.h>
#include <wx/imaglist.h>
#include <wx/bitmap.h>
#include <wx/fs_mem.h>
#include <wx/utils.h>

#include <wx/busyinfo.h>


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

const wxChar *greeting = wxT("<html><head><title>About</title></head>")
        wxT("<body><table border=0><tr><td align=\"left\">")
        wxT("<img src=\"memory:logo.xpm\"></td><td align=\"left\">")
	wxT("Hello, and welcome to <B>xCHM</B>, the UNIX CHM viewer.")
	wxT("<br><br><B>xCHM</B> has been written by Razvan Cojocaru ")
	wxT("(rzvncj@gmail.com). It is licensed under the <TT>GPL</TT>.<br>")
	wxT("<B>xCHM</B> is based on Jed Wing's <a href=\"http://www.jedrea.com/")
	wxT("chmlib/\">CHMLIB</a> and <a href=\"http://www.")
	wxT("wxwidgets.org\">wxWidgets</a>.</td></tr></table>")
	wxT("<br>If you'd like to know more about CHM, go to")
	wxT(" <a href=\"http://www.nongnu.org/chmspec/latest/\">Pabs' CHM")
	wxT(" Specification page</a>.<br>Pabs has contributed time and knowledge")
	wxT(" to the development of <B>xCHM</B>, and features such as the fast")
	wxT(" index search would not have been implemented without his help.")
	wxT(" Portions of the fast index search are modified versions of")
	wxT(" Pabs' <TT>GPL</TT>d <TT>chmdeco</TT> code.<br><br>If")
	wxT(" you'd like to use the code in your own stuff please figure")
	wxT(" <TT>GPL</TT> out first.")
#if !defined(wxUSE_UNICODE) || !wxUSE_UNICODE
	wxT("<br><br><B>WARNING</B>: your <B>xCHM</B> binary is linked")
	wxT(" against a non-Unicode version of wxWidgets! While it will")
	wxT(" work with most CHMs, it might not properly display")
	wxT(" special character languages.")
#endif
	wxT("<br><br>Tips:<br><ul><li>")
	wxT("The global search is an \'AND\' search, i.e. searching for")
	wxT(" \'word1 word2\' (quotes not included) will find all the pages")
	wxT(" that contain both word1 and word2, in whatever order.</li>")
	wxT("<li>Allowing partial matches will find pages that contain")
	wxT(" words that only start with the typed strings, i.e. searching")
	wxT(" for \'ans 4\' (quotes not included) will find pages that")
	wxT(" contain the sentence \'The answer is 42.\'.</li><li>Right")
	wxT(" clicking on the displayed page brings out a popup menu with")
	wxT(" common options.</li></ul><br><br>Ctrl(cmd)-C is copy,")
	wxT(" Ctrl(cmd)-F is find in page.<br><br>Enjoy.</body></html>");


const wxChar *error_page =
	wxT("<html><body>Error loading CHM file!</body></html>");


const wxChar *about_txt = wxT("xCHM v. ") wxT(VERSION) 
	wxT("\nby Razvan Cojocaru <rzvncj@gmail.com>\n\n")
	wxT("With thanks to Pabs (http://bonedaddy.net/pabs3/hhm/).\n")
	wxT("Based on Jed Wing's CHMLIB (http://66.93.236.84/~jedwin/projects/).\n")
	wxT("XMLRPC code for context sensitive help contributed by\n")
	wxT("Eamon Millman <millman@pcigeomatics.com>.\n")
	wxT("<SPAN> tag support and contents tree icons contributed by\n")
	wxT("Fritz Elfert <felfert@users.sourceforge.net>.\n")
	wxT("Tabbed browsing support contributed by Cedric Boudinet")
	wxT(" <bouced@gmx.fr>.\n")
	wxT("Written with wxWidgets (http://www.wxwidgets.org).\n\n")
	wxT("This program is (proudly) under the GPL.");

#include <xchm-32.xpm>
#include <logo.xpm>

} // namespace


CHMFrame::CHMFrame(const wxString& title, const wxString& booksDir, 
		   const wxPoint& pos, const wxSize& size,
		   const wxString& normalFont, const wxString& fixedFont,
		   const int fontSize, const int sashPosition,
		   const wxString& fullAppPath, bool loadTopics,
		   bool loadIndex)
	: wxFrame(NULL, -1, title, pos, size),
	  _tcl(NULL), _sw(NULL), _menuFile(NULL), _tb(NULL), _ep(NULL),
	  _nb(NULL), _cb(NULL), _csp(NULL), _cip(NULL), _openPath(booksDir), 
	  _normalFonts(NULL), _fixedFonts(NULL), _normalFont(normalFont),
	  _fixedFont(fixedFont), _fontSize(fontSize), _bookmarkSel(true),
	  _bookmarksDeleted(false), _sashPos(sashPosition),
	  _fullAppPath(fullAppPath), _loadTopics(loadTopics),
	  _loadIndex(loadIndex), _fullScreen(false)
{
#	if wxUSE_ACCEL
	const int NO_ACCELERATOR_ENTRIES = 6;
	wxAcceleratorEntry entries[NO_ACCELERATOR_ENTRIES];

	entries[0].Set(wxACCEL_CTRL, (int) 'F', ID_FindInPage);
	entries[1].Set(wxACCEL_CTRL, (int) 'C', ID_CopySelection);
	entries[2].Set(wxACCEL_CTRL, (int) ']', ID_Forward);
	entries[3].Set(wxACCEL_CTRL, (int) '[', ID_Back);
	entries[4].Set(wxACCEL_CTRL, WXK_F4, ID_CloseTab);
	entries[5].Set(wxACCEL_CTRL, 'Q', ID_Quit);

	wxAcceleratorTable accel(NO_ACCELERATOR_ENTRIES, entries);
	SetAcceleratorTable(accel);
#	endif

#ifdef __WXMAC__	
	wxApp::s_macAboutMenuItemId = ID_About;
#endif

	wxLogNull wln;
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
	_nbhtml = new CHMHtmlNotebook(_sw, _tcl, _normalFont, _fixedFont,
				      fontSize, this);
	_nbhtml->SetChildrenFonts(_normalFont, _fixedFont, sizes);

	_csp = new CHMSearchPanel(_nb, _tcl, _nbhtml);
	_font = _tcl->GetFont();

	_cip = new CHMIndexPanel(_nb, _nbhtml);

	_nb->AddPage(temp, _("Contents"));
	_nb->AddPage(_cip, _("Index"));
	_nb->AddPage(_csp, _("Search"));

	_sw->Initialize(_nbhtml);
	_nbhtml->GetCurrentPage()->SetFocusFromKbd();
}


CHMFrame::~CHMFrame()
{
	if(_tcl) // Supposedly, workaround for wxWin
		_tcl->Unselect();

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
				 wxT("CHM files (*.chm)|*.chm;*.CHM|")
				 wxT("All files (*.*)|*.*"), 
#else
				 wxT("All files (*.*)|*.*"),
#endif
				 wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

	if(selection.IsEmpty() || !_tcl)
		return;

	_openPath = selection.BeforeLast(wxT('/'));
	LoadCHM(selection);
}


void CHMFrame::OnChangeFonts(wxCommandEvent& WXUNUSED(event))
{
	wxLogNull wln;

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

		_nbhtml->SetChildrenFonts(_normalFont = cfd.NormalFont(), 
				_fixedFont = cfd.FixedFont(),
				cfd.Sizes());

		_fontSize = *(cfd.Sizes() + 3);

		wxString page = _nbhtml->GetCurrentPage()->GetOpenedPage();

		if(page.IsEmpty())
			_nbhtml->GetCurrentPage()
				->LoadPage(wxT("memory:about.html"));
	}
}


void CHMFrame::OnHome(wxCommandEvent& WXUNUSED(event))
{
	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	_nbhtml->LoadPageInCurrentView(wxString(wxT("file:")) +
				       chmf->ArchiveName()
				       + wxT("#xchm:") + chmf->HomePage());
}


void CHMFrame::OnHistoryForward(wxCommandEvent& WXUNUSED(event))
{
	_nbhtml->GetCurrentPage()->HistoryForward();
}


void CHMFrame::OnHistoryBack(wxCommandEvent& WXUNUSED(event))
{
	if(_nbhtml->GetCurrentPage()->HistoryCanBack())
		_nbhtml->GetCurrentPage()->HistoryBack();
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
			_sw->SplitVertically(_nb, _nbhtml, _sashPos);

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

#	define EC_WARNING_MSG "This is experimental code, and \
doing this will overwrite any previously registered CHM viewer \
associations.\n\nAre you sure you know what you're doing?"

void CHMFrame::OnRegisterExtension(wxCommandEvent& WXUNUSED(event))
{
	int answer = wxMessageBox(_(EC_WARNING_MSG), _("Confirm"),
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
	wxLogNull wln;

        int sizes[7];
	        for(int i = -3; i <= 3; ++i)
	                sizes[i+3] = _fontSize + i * 2;
			
	_ep->SetFonts(_normalFont, _fixedFont, sizes);
	_ep->PrintFile(_nbhtml->GetCurrentPage()->GetOpenedPage());
}


void CHMFrame::OnHistFile(wxCommandEvent& event)
{
      wxString f(_fh.GetHistoryFile(event.GetId() - wxID_FILE1));
      if (!f.IsEmpty())
	      LoadCHM(f);
}


void CHMFrame::OnFind(wxCommandEvent& event)
{
	_nbhtml->GetCurrentPage()->OnFind(event);
}


void CHMFrame::OnCloseTab(wxCommandEvent& event)
{
	_nbhtml->OnCloseTab(event);
}


void CHMFrame::OnNewTab(wxCommandEvent& event)
{
	_nbhtml->OnNewTab(event);
}


void CHMFrame::OnCopySelection(wxCommandEvent& event)
{
	_nbhtml->GetCurrentPage()->OnCopy(event);
}


void CHMFrame::OnFullScreen(wxCommandEvent& WXUNUSED(event))
{
	_fullScreen = !_fullScreen;
	ShowFullScreen(_fullScreen, wxFULLSCREEN_ALL);
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

	_nbhtml->LoadPageInCurrentView(wxString(wxT("file:")) + 
				       chmf->ArchiveName() +
				       wxT("#xchm:/") + *url);
}


void CHMFrame::OnSelectionChanged(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	CHMFile *chmf = CHMInputStream::GetCache();

	if(id == _tcl->GetRootItem() || !chmf)
		return;

	if(!id.IsOk())
		return;

	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(id));

	if(!data || data->_url.IsEmpty())
		return;

	if(!_nbhtml->GetCurrentPage()->IsCaller()) {
		_nbhtml->GetCurrentPage()->SetSync(false);
		_nbhtml->LoadPageInCurrentView(wxString(wxT("file:")) + 
					       chmf->ArchiveName() +
					       wxT("#xchm:/") + data->_url);
		_nbhtml->GetCurrentPage()->SetSync(true);
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
	_nbhtml->CloseAllPagesExceptFirst();
	if(!archive.StartsWith(wxT("file:")) || 
	   !archive.Contains(wxT("#xchm:"))) {

		wxFileSystem wfs;
		std::auto_ptr<wxFSFile> p(wfs.OpenFile(wxString(wxT("file:")) + archive +
			      wxT("#xchm:/")));
	
	        CHMFile *chmf = CHMInputStream::GetCache();

		if(!chmf)
			return false;
	
		rtn = _nbhtml->LoadPageInCurrentView(wxString(wxT("file:"))
						+ chmf->ArchiveName()
				                + wxT("#xchm:") 
						+ chmf->HomePage());
	} else {
		rtn = _nbhtml->LoadPageInCurrentView(archive);
	}

	if(!rtn) { // Error, could not load CHM file
		if(_tcl->GetCount())
			_tcl->Unselect();
			_tcl->DeleteChildren(_tcl->GetRootItem());
		if(_sw->IsSplit()) {
			_sw->Unsplit(_nb);
			_nb->Show(FALSE);
		}
		_menuFile->Check(ID_Contents, FALSE);
		_tb->ToggleTool(ID_Contents, FALSE);
		_cip->Reset();
		_csp->Reset();
		_nbhtml->LoadPageInCurrentView(wxT("memory:error.html"));

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

	return _nbhtml->LoadPageInCurrentView(wxString(wxT("file:")) + 
					      chmf->ArchiveName()
					      + wxT("#xchm:") + 
					      chmf->GetPageByCID(contextID));
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

	wxWindowDisabler wwd;
	wxBusyInfo wait(_("Loading, please wait.."), this);

	wxString filename = chmf->ArchiveName();
	if(!filename.IsEmpty()) {		
		_fh.AddFileToHistory(filename);

		if(!_menuFile->IsEnabled(ID_Recent))
			_menuFile->Enable(ID_Recent, TRUE);
	}

	_nbhtml->GetCurrentPage()->HistoryClear();
	_csp->Reset();
	_cip->Reset();

	wxString title = chmf->Title();

	if(_tcl->GetCount()) {
		_tcl->Unselect();
		_tcl->DeleteChildren(_tcl->GetRootItem());
	}

#if !wxUSE_UNICODE
	wxString fontFace = chmf->DefaultFont();

	if(!fontFace.IsEmpty()) {

		long fs = -1;		
		fontFace.BeforeLast(wxT(',')).AfterLast(wxT(',')).ToLong(&fs);

		if(fs < 10)
			fs = 10;

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
			_nbhtml->SetChildrenFonts(font.GetFaceName(),
						  font.GetFaceName(),
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

		_nbhtml->SetChildrenFonts(_normalFont, _fixedFont, sizes);
		noSpecialFont = true;
	}
#endif
	if(_loadTopics)
		chmf->GetTopicsTree(_tcl);
	
	if(_loadIndex)
		chmf->GetIndex(_cip->GetResultsList());
	
	if(!title.IsEmpty()) {
		wxString titleBarText = 
			wxString(wxT("xCHM v. " wxT(VERSION) wxT(": "))) 
                        + title;

		SetTitle(titleBarText);
	} else {
		SetTitle(wxT("xCHM v. ") wxT(VERSION));
	}
	
	// if we have contents..
	if(_tcl->GetCount() >= 1) {		
		if(!_sw->IsSplit()) {
			_nb->Show(TRUE);
			_sw->SplitVertically(_nb, _nbhtml, _sashPos);
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
	_menuFile->Append(ID_Fonts, _("Fon&ts.."), FONTS_HELP);
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
	menuHistory->Append(ID_Forward, _("For&ward\tAlt-RIGHT"), 
			    FORWARD_HELP);
	menuHistory->Append(ID_Back, _("&Back\tAlt-LEFT"), BACK_HELP);
	
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(ID_About, _("&About..\tF1"), ABOUT_HELP);

	wxMenu *menuEdit = new wxMenu;
	menuEdit->Append(ID_CopySelection, _("&Copy\tCtrl-C"), COPY_HELP);
	menuEdit->Append(ID_FindInPage, _("&Find..\tCtrl-F"), FIND_HELP);
	menuEdit->AppendSeparator();
	menuEdit->Append(ID_CloseTab, _("&Close tab\tCtrl-W"), CLOSETAB_HELP);
	menuEdit->Append(ID_NewTab, _("&New tab\tCtrl-T"), NEWTAB_HELP);

	wxMenu *menuView = new wxMenu;
	menuView->Append(ID_FullScreen, _("Toggle &fullscreen\tF11"),
			 FULLSCREEN_HELP);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(_menuFile, _("&File"));
	menuBar->Append(menuView, _("&View"));
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

	wxImageList *il = new wxImageList(16, 16);
	il->Add(wxIcon(hbook_closed_xpm));
	il->Add(wxIcon(hbook_open_xpm));
	il->Add(wxIcon(hpage_xpm));

	_tcl = new wxTreeCtrl(temp, ID_TreeCtrl, wxDefaultPosition, 
			      wxDefaultSize, 
			      wxSUNKEN_BORDER | wxTR_HIDE_ROOT
			      | wxTR_LINES_AT_ROOT);

	_tcl->AssignImageList(il);	
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
#include <fullscreen.xpm>

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
	
	toolbar->AddTool(ID_FullScreen, _("Fullscreen"),
			 wxBitmap(fullscreen_xpm), FULLSCREEN_HELP);

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


void CHMFrame::AddHtmlView(const wxString& path, const wxString& link)
{
	_nbhtml->AddHtmlView(path, link);
}


void CHMFrame::ToggleFullScreen(bool onlyIfFullScreenOn)
{
	if(onlyIfFullScreenOn && !_fullScreen)
		return;

	wxCommandEvent dummy;
	OnFullScreen(dummy);
}


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
	EVT_MENU(ID_CloseTab, CHMFrame::OnCloseTab)
	EVT_MENU(ID_NewTab, CHMFrame::OnNewTab)
	EVT_MENU(ID_CopySelection, CHMFrame::OnCopySelection)
	EVT_MENU(ID_FullScreen, CHMFrame::OnFullScreen)
	EVT_BUTTON(ID_Add, CHMFrame::OnAddBookmark)
	EVT_BUTTON(ID_Remove, CHMFrame::OnRemoveBookmark)
	EVT_TREE_SEL_CHANGED(ID_TreeCtrl, CHMFrame::OnSelectionChanged)
	EVT_COMBOBOX(ID_Bookmarks, CHMFrame::OnBookmarkSel)
	EVT_TEXT_ENTER(ID_Bookmarks, CHMFrame::OnBookmarkSel)
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

