/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
 
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
#include <contenttaghandler.h>


namespace {

const char *greeting =
	"<html><body>Hello, and welcome to "
	"<B>xCHM</B>, the UNIX CHM viewer.<br><br><B>"
	"xCHM</B> has been written by Razvan Cojocaru "
	"(razvanco@gmx.net). It is licensed under the "
	"<TT>GPL</TT>.<br><B>xCHM</B> is based on"
	" Jed Wing's <a href=\"http://66.93.236.84/~jedwin/projects"
	"/chmlib/\">CHMLIB</a> and <a href=\"http://www.wxwindows.org\">"
	"wxWindows</a>.<br><br>If you'd like to know more about CHM, you"
	" could check out <a href=\"http://www.speakeasy.org/~russotto"
	"/chm/\">Matthew Russoto's CHM page</a> or <a"
	" href=\"http://bonedaddy.net/pabs3/hhm/\">Pabs' CHM"
	" Specification page</a>.<br><br>If you'd like to use the code in"
	" your own stuff please figure <TT>GPL</TT> out first. Far too"
	" many people think <TT>GPL</TT>"
	" is bad out of utter ignorance.<br><br>Enjoy.</body></html>";


const char *about_txt = 
	"xCHM v. " VERSION "\nby Razvan Cojocaru (razvanco@gmx.net)\n\n"
	"Based on Jed Wing's CHMLIB (http://66.93.236.84/~jedwin/projects).\n"
	"Written with wxWindows (http://www.wxwindows.org).\n\n"
	"This program is (proudly) under the GPL.";

#include <htmbook.xpm>

} // namespace


CHMFrame::CHMFrame(const wxString& title, const wxString& booksDir, 
		   const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, -1, title, pos, size), _html(NULL),
	  _tcl(NULL), _sw(NULL), _menuFile(NULL), _tb(NULL), _ep(NULL),
	  _openPath(booksDir)
{
	int sizes[] = {8, 10, 12, 14, 16, 18, 20};

	SetIcon(wxIcon(htmbook_xpm));
	SetMenuBar(CreateMenu());

	_tb = CreateToolBar(wxTB_FLAT | wxTB_DOCKABLE
			    | wxTB_HORIZONTAL | wxTB_TEXT);
	InitToolBar(_tb);

	CreateStatusBar();
	SetStatusText( "Ready." );

	_ep = new wxHtmlEasyPrinting("Printing", this);

	_sw = new wxSplitterWindow(this);
	_sw->SetMinimumPaneSize(100);

	_html = new wxHtmlWindow(_sw, -1,  wxDefaultPosition, wxSize(200,200));
	_html->SetRelatedFrame(this, wxString("xCHM v. ") + VERSION);
	_html->SetRelatedStatusBar(0);

	_html->SetFonts("", "", sizes);
	_html->SetPage(greeting);

	_tcl = new wxTreeCtrl(_sw, ID_TreeCtrl);
	_tcl->Show(FALSE);

	_sw->Initialize(_html);
}


CHMFrame::~CHMFrame()
{
	delete _ep;
}


void CHMFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}


void CHMFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	::wxMessageBox(about_txt, 
		       "About xCHM", 
		       wxOK | wxICON_INFORMATION, this );
}


void CHMFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	wxString selection =
		::wxFileSelector("Choose a file..", _openPath, "", "chm",
#ifndef __WXMOTIF__
				 // they say Motif can't handle the following.
				 "CHM files (*.chm)|*.chm;*.CHM|"
				 "All files (*.*)|*.*", 
#else
				 "All files (*.*)|*.*",
#endif
				 wxOPEN | wxFILE_MUST_EXIST, this);

	if(selection.IsEmpty() || !_tcl)
		return;

	_openPath = selection.BeforeLast('/');
	LoadCHM(selection);
}


void CHMFrame::OnHome(wxCommandEvent& WXUNUSED(event))
{
	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	_html->LoadPage(wxString("file:") + chmf->ArchiveName()
		+ "#chm:" + chmf->HomePage());
}


void CHMFrame::OnHistoryForward(wxCommandEvent& WXUNUSED(event))
{
	_html->HistoryForward();
}


void CHMFrame::OnHistoryBack(wxCommandEvent& WXUNUSED(event))
{
	_html->HistoryBack();
}


void CHMFrame::OnShowContents(wxCommandEvent& WXUNUSED(event))
{
	if(_sw->IsSplit()) {
		_tb->ToggleTool(ID_Contents, FALSE);
		_menuFile->Check(ID_Contents, FALSE);
		
		_sw->Unsplit(_tcl);
		_tcl->Show(FALSE);
	} else {		
			
		if(_tcl->GetCount() > 1) {
			
			_tb->ToggleTool(ID_Contents, TRUE);
			_menuFile->Check(ID_Contents, TRUE);

			_tcl->Show(TRUE);
			_sw->SplitVertically(_tcl, _html, 150);

		} else {
			_tb->ToggleTool(ID_Contents, FALSE);
			_menuFile->Check(ID_Contents, FALSE);
			
			::wxMessageBox("Couldn't extract the book"
				       " contents tree.", 
				       "No contents..", 
				       wxOK | wxICON_WARNING, this );
		}
	}
}


void CHMFrame::OnPrint(wxCommandEvent& WXUNUSED(event))
{
	wxString page = _html->GetOpenedPage();

	if(page.IsEmpty())
		_ep->PrintText(greeting);
	else
		_ep->PrintFile(page);
}


void CHMFrame::OnSelectionChanged(wxTreeEvent& event)
{
	if(!_tcl)
		return;

	wxTreeItemId id = event.GetItem();
	CHMFile *chmf = CHMInputStream::GetCache();

	if(id == _tcl->GetRootItem() || !chmf)
		return;

	URLTreeItem *data = (URLTreeItem *)_tcl->GetItemData(id);

	if(!data || data->_url.IsEmpty())
		return;

	_html->LoadPage(wxString("file:") + chmf->ArchiveName() +
			"#chm:/" + data->_url);
}


void CHMFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
	int xorig, yorig, width, height;

	GetPosition(&xorig, &yorig);
	GetSize(&width, &height);

	wxConfig config("xchm");
	
	config.Write("/Position/xOrig", xorig);
	config.Write("/Position/yOrig", yorig);
	config.Write("/Position/width", width);
	config.Write("/Position/height", height);
	config.Write("/Paths/lastOpenedDir", _openPath);

	Destroy();
}


void CHMFrame::LoadCHM(const wxString& archive)
{
	_html->HistoryClear();
	_html->LoadPage(wxString("file:") + archive +
		      wxString("#chm:/"));

	CHMFile *chmf = CHMInputStream::GetCache();

	if(chmf) {
		wxString title = chmf->Title();

		if(_tcl->GetCount())
			_tcl->DeleteAllItems();
		chmf->GetTopicsTree(_tcl);

		if(!title.IsEmpty()) {
			wxString titleBarText = wxString("xCHM v. ") 
				+ VERSION
				+ ": " + title;

			SetTitle(titleBarText);
			_html->SetRelatedFrame(this, titleBarText);
		}
	}

	// if we have contents..
	if(_tcl->GetCount() > 1) {		
		if(!_sw->IsSplit()) {
			_tcl->Show(TRUE);
			_sw->SplitVertically(_tcl, _html, 150);
			_menuFile->Check(ID_Contents, TRUE);
			_tb->ToggleTool(ID_Contents, TRUE);
		}
	} else {

		if(_sw->IsSplit()) {
			_sw->Unsplit(_tcl);
			_tcl->Show(FALSE);
		}
		_menuFile->Check(ID_Contents, FALSE);
		_tb->ToggleTool(ID_Contents, FALSE);
	}

}


namespace {

const char *open_help = "Open a CHM book.";
const char *print_help = "Print the page currently displayed";
const char *contents_help = "On or off?";
const char *home_help = "Go to the book's start page.";
const char *forward_help = "Go forward in history. Per book.";
const char *back_help = "Back to the last visited page. Per book.";
const char *about_help = "About the program.";

} // namespace


wxMenuBar* CHMFrame::CreateMenu()
{
	_menuFile = new wxMenu;

	_menuFile->Append(ID_Open, "&Open ..", open_help);
	_menuFile->Append(ID_Print, "&Print page..", print_help);
	_menuFile->AppendSeparator();
	_menuFile->AppendCheckItem(ID_Contents, "&Show contents tree",
				   contents_help);
	_menuFile->AppendSeparator();
	_menuFile->Append(ID_Quit, "E&xit", "Quit the application.");

	wxMenu *menuHistory = new wxMenu;

	menuHistory->Append(ID_Home, "&Home", home_help);
	menuHistory->Append(ID_Forward, "&Forward", forward_help);
	menuHistory->Append(ID_Back, "&Back", back_help);
	
	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(ID_About, "&About ..", about_help);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(_menuFile, "&File");
	menuBar->Append(menuHistory, "Hi&story");
	menuBar->Append(menuHelp, "&Help");

	return menuBar;
}


namespace {

#include <fileopen.xpm>
#include <print.xpm>
#include <back.xpm>
#include <forward.xpm>
#include <home.xpm>
#include <helpicon.xpm>
#include <htmsidep.xpm>

} // namespace


bool CHMFrame::InitToolBar(wxToolBar *toolbar)
{
	toolbar->AddTool(ID_Open, "Open ..", wxBitmap(fileopen_xpm),
			 open_help);
	toolbar->AddTool(ID_Print, "Print ..", wxBitmap(print_xpm),
			 print_help);
	toolbar->AddCheckTool(ID_Contents, "Contents", 
			      wxBitmap(htmsidep_xpm),
			      wxBitmap(htmsidep_xpm), contents_help);
	toolbar->AddSeparator();
	toolbar->AddTool(ID_Back, "Back", wxBitmap(back_xpm), back_help);
	toolbar->AddTool(ID_Forward, "Forward", wxBitmap(forward_xpm), 
			 forward_help);
	toolbar->AddTool(ID_Home, "Home", wxBitmap(home_xpm), home_help);
	toolbar->AddSeparator();
	toolbar->AddTool(ID_About, "About", wxBitmap(helpicon_xpm), 
			 about_help);

	toolbar->Realize();

	return TRUE;
}


// They say this needs to be in the implementation file.
BEGIN_EVENT_TABLE(CHMFrame, wxFrame)
	EVT_MENU(ID_Quit,  CHMFrame::OnQuit)
	EVT_MENU(ID_About, CHMFrame::OnAbout)
	EVT_MENU(ID_Open, CHMFrame::OnOpen)
	EVT_MENU(ID_Home, CHMFrame::OnHome)
	EVT_MENU(ID_Forward, CHMFrame::OnHistoryForward)
	EVT_MENU(ID_Back, CHMFrame::OnHistoryBack)
	EVT_MENU(ID_Contents, CHMFrame::OnShowContents)
	EVT_MENU(ID_Print, CHMFrame::OnPrint)
	EVT_TREE_SEL_CHANGED(ID_TreeCtrl, CHMFrame::OnSelectionChanged)
	EVT_CLOSE(CHMFrame::OnCloseWindow)
END_EVENT_TABLE()
