/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
  Mac OS specific patches contributed by Chanler White 
  <cawhite@nwrails.com>
 
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


#include <chmhtmlwindow.h>
#include <contenttaghandler.h>
#include <chminputstream.h>
#include <chmframe.h>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/clipbrd.h>



CHMHtmlWindow::CHMHtmlWindow(wxWindow *parent, wxTreeCtrl *tc,
			     CHMFrame *frame)
	: wxHtmlWindow(parent, -1, wxDefaultPosition, wxSize(200,200)),
	  _tcl(tc), _syncTree(true), _found(false), _menu(NULL), 
	  _absolute(false), _frame(frame), _link(NULL)
#ifdef _ENABLE_COPY_AND_FIND
	, _fdlg(NULL)
#endif
{
	_menu = new wxMenu;
	_menu->Append(ID_PopupForward, _("For&ward"));
	_menu->Append(ID_PopupBack, _("&Back"));
	_menu->Append(ID_CopyLink, _("Copy &link location"));

#ifdef _ENABLE_COPY_AND_FIND
	_menu->AppendSeparator();
	_menu->Append(ID_CopySel, _("&Copy selection"));
	_menu->AppendSeparator();
	_menu->Append(ID_PopupFind, _("&Find in page.."));
#endif
}


CHMHtmlWindow::~CHMHtmlWindow()
{
	delete _menu;
#ifdef _ENABLE_COPY_AND_FIND
	delete _fdlg;
#endif
}


bool CHMHtmlWindow::LoadPage(const wxString& location)
{
	wxLogNull log;
	wxString tmp = location;

	// Path is already absolute.
	if(_absolute) {
		FixRelativePath(tmp, wxEmptyString);
		_absolute = false;
	} else
		FixRelativePath(tmp, GetPrefix(GetOpenedPage()));

	if(!tmp.Left(19).CmpNoCase(wxT("javascript:fullsize"))) 
		tmp = tmp.AfterFirst(wxT('\'')).BeforeLast(wxT('\''));

	_prefix = GetPrefix(tmp);

	if(_syncTree && 
	   // We should be looking for a valid page, not / (home).
	   !location.AfterLast(wxT('/')).IsEmpty() && 
	   _tcl->GetCount() > 1) {

		wxString srch;
		if(tmp.StartsWith(wxT("/")) || tmp.StartsWith(wxT("file:")))
			srch = tmp.AfterLast(wxT(':')).AfterFirst(
				wxT('/')).BeforeFirst(wxT('#'));
		else
			srch = tmp.BeforeFirst(wxT('#'));
		
		// Sync will call SelectItem() on the tree item
		// if it finds one, and that in turn will call
		// LoadPage() with _syncTree set to false.
		Sync(_tcl->GetRootItem(), srch);

		if(_found)
			_found = false;
	}

	return wxHtmlWindow::LoadPage(tmp);
}


void CHMHtmlWindow::Sync(wxTreeItemId root, const wxString& page)
{
	if(_found)
		return;

	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(root));

	wxString url;

	if(data)
		url = (data->_url).BeforeFirst(wxT('#'));

	if(data && (!url.CmpNoCase(page) || 
		    !url.CmpNoCase(GetPrefix(GetOpenedPage()) + wxT("/") 
				   + page))) {
		// Order counts!
		_found = true;
		_tcl->SelectItem(root);
		return;
	}

#ifndef _ENABLE_COPY_AND_FIND
	long cookie;
#else
	wxTreeItemIdValue cookie;
#endif
	wxTreeItemId child = _tcl->GetFirstChild(root, cookie);

	for(size_t i = 0; i < _tcl->GetChildrenCount(root, FALSE); ++i) {
		Sync(child, page);
		child = _tcl->GetNextChild(root, cookie);
	}
}


inline 
wxString CHMHtmlWindow::GetPrefix(const wxString& location) const
{
	return location.AfterLast(wxT(':')).AfterFirst(
		wxT('/')).BeforeLast(wxT('/'));
}


bool CHMHtmlWindow::FixRelativePath(wxString &location,
				    const wxString& prefix) const
{
	if(!location.Left(5).CmpNoCase(wxT("file:")) || 
	   !location.Left(5).CmpNoCase(wxT("http:")) ||
	   !location.Left(6).CmpNoCase(wxT("https:")) ||
	   !location.Left(4).CmpNoCase(wxT("ftp:")) ||
	   !location.Left(7).CmpNoCase(wxT("mailto:")) ||
	   !location.Left(7).CmpNoCase(wxT("ms-its:")) ||
	   !location.Left(10).CmpNoCase(wxT("javascript")) ||
	   location.StartsWith(wxT("#")))
		return false;
	  
	CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return false;

	bool result = location.StartsWith(wxT(".."));
	wxString prf = prefix;

	while(location.StartsWith(wxT(".."))) {
		location = location.AfterFirst(wxT('/'));
		prf = prf.BeforeLast(wxT('/'));
	}

	if(location.StartsWith(wxT("./")))
		location = location.AfterFirst(wxT('/'));
		
	if(!prf.IsEmpty())
		prf = wxString(wxT("/")) + prf;

	// Handle absolute paths too (that start with /)
	if(!location.StartsWith(wxT("/")))
		location = wxString(wxT("file:")) + chmf->ArchiveName() + 
			wxT("#chm:") + prf + wxT("/") + location;
	else
		location = wxString(wxT("file:")) + chmf->ArchiveName() + 
			wxT("#chm:") + location;

	// Don't redirect for simple relative paths that wxHTML can
	// handle by itself.
	return result;
}


wxHtmlOpeningStatus CHMHtmlWindow::OnOpeningURL(wxHtmlURLType type,
						const wxString& url, 
						wxString *redirect) const
{
	if(type == wxHTML_URL_PAGE)
		return wxHTML_OPEN;

	wxString tmp = url;

	if(FixRelativePath(tmp, _prefix)) {
		*redirect = tmp;
		return wxHTML_REDIRECT;
	}

	return wxHTML_OPEN;
}


void CHMHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	wxString url = link.GetHref();

	LoadPage(url);

	if(!url.Left(7).CmpNoCase(wxT("MS-ITS:")))
		_frame->UpdateCHMInfo();
}


#ifdef _ENABLE_COPY_AND_FIND

wxHtmlCell* CHMHtmlWindow::FindFirst(wxHtmlCell *parent, const wxString& word,
				     bool wholeWords, bool caseSensitive)
{
	wxString tmp = word;
	
	if(!parent)
		return NULL;
 
	if(!caseSensitive)
		tmp.MakeLower();

	// If this cell is not a container, the for body will never happen
	// (GetFirstChild() will return NULL).
	for(wxHtmlCell *cell = parent->GetFirstChild(); cell; 
	    cell = cell->GetNext()) {
		
		wxHtmlCell *result;
		if((result = FindFirst(cell, word, wholeWords, caseSensitive)))
			return result;
	}

	wxHtmlSelection ws;
	ws.Set(parent, parent);
	wxString text = parent->ConvertToText(&ws);

	if(text.IsEmpty())
		return NULL;

	if(!caseSensitive)
		text.MakeLower();

	text.Trim(TRUE);
	text.Trim(FALSE);

	bool found = false;

	if(wholeWords && text == tmp) {
		found = true;
	} else if(!wholeWords && text.Find(tmp.c_str()) != -1) {
		found = true;
	}

	if(found) {
		// What is all this wxWidgets protected member crap?
		delete m_selection;
		m_selection = new wxHtmlSelection();

		// !! Must see if this works now. !!
		m_selection->Set(parent, parent);		

		int y;
		wxHtmlCell *cell = parent;

		for (y = 0; cell != NULL; cell = cell->GetParent()) 
			y += cell->GetPosY();
		Scroll(-1, y / wxHTML_SCROLL_STEP);
		Refresh();
		
		return parent;
	}

	return NULL;
}


wxHtmlCell* CHMHtmlWindow::FindNext(wxHtmlCell *start, const wxString& word, 
				    bool wholeWords, bool caseSensitive)
{
	wxHtmlCell *cell;

	if(!start)
		return NULL;

	for(cell = start; cell; cell = cell->GetNext()) {
		wxHtmlCell *result;
		if((result = FindFirst(cell, word, wholeWords, caseSensitive)))
			return result;
	}

	cell = start->GetParent();
	
	while(cell && !cell->GetNext())
		cell = cell->GetParent();

	if(!cell)
		return NULL;

	return FindNext(cell->GetNext(), word, wholeWords, caseSensitive);
}


void CHMHtmlWindow::ClearSelection()
{
	delete m_selection;
	m_selection = NULL;
	Refresh();
}

void CHMHtmlWindow::OnCopy(wxCommandEvent& WXUNUSED(event))
{
	CopySelection();
}


void CHMHtmlWindow::OnFind(wxCommandEvent& WXUNUSED(event))
{
	if(!_fdlg) {
		wxWindow* p = GetParent();
		while(p->GetParent())
			p = p->GetParent();

		_fdlg = new CHMFindDialog(p, this);
	}

	_fdlg->CentreOnParent();
	_fdlg->ShowModal();
	_fdlg->SetFocusToTextBox();
	_fdlg->Reset();
}

#endif // _ENABLE_COPY_AND_FIND


void CHMHtmlWindow::OnForward(wxCommandEvent& WXUNUSED(event))
{
	HistoryForward();
}


void CHMHtmlWindow::OnBack(wxCommandEvent& WXUNUSED(event))
{
	HistoryBack();
}


void CHMHtmlWindow::OnCopyLink(wxCommandEvent& WXUNUSED(event))
{
	if(wxTheClipboard->Open()) {
		wxTheClipboard->SetData(
			new wxTextDataObject(_link->GetHref()));
		wxTheClipboard->Close();
	}
}


void CHMHtmlWindow::OnRightClick(wxMouseEvent& event)
{
#ifdef _ENABLE_COPY_AND_FIND
	if(IsSelectionEnabled())
		_menu->Enable(ID_CopySel, m_selection != NULL);
#endif

	_menu->Enable(ID_PopupForward, HistoryCanForward());
	_menu->Enable(ID_PopupBack, HistoryCanBack());
	_menu->Enable(ID_CopyLink, false);

	wxHtmlCell *cell = 
		m_Cell->FindCellByPos(event.m_x, event.m_y);

	if(cell) {
		_link = cell->GetLink();

		if(_link) {
			_menu->Enable(ID_CopyLink, true);
		}
	}

	PopupMenu(_menu, event.GetPosition());
}


#ifdef __WXMAC__
/* This is a hack, consisting of copying the relevent event handler from
   the wxWidgets src/generic/scrlwing.cpp file to make up for the fact that for
   some reason, compiling wxWidgets with scroll wheel support is exposing a 
   crashing bug on OS X.
   When this gets fixed in wxWidgets, this should prob be just removed.. */
void CHMHtmlWindow::HandleOnMouseWheel(wxMouseEvent& event)
{
	int m_wheelRotation = 0;
	m_wheelRotation += event.GetWheelRotation();
	int lines = m_wheelRotation / event.GetWheelDelta();
	m_wheelRotation -= lines * event.GetWheelDelta();

	if (lines != 0)
	{
		wxScrollWinEvent newEvent;

		newEvent.SetPosition(0);
		newEvent.SetOrientation(wxVERTICAL);
		newEvent.m_eventObject = m_win;

		if (event.IsPageScroll())
		{
			if (lines > 0)
				newEvent.m_eventType = 
					wxEVT_SCROLLWIN_PAGEUP;
			else
				newEvent.m_eventType = 
					wxEVT_SCROLLWIN_PAGEDOWN;

			m_win->GetEventHandler()->ProcessEvent(newEvent);
		} else {
			lines *= event.GetLinesPerAction();
			if (lines > 0)
				newEvent.m_eventType = 
					wxEVT_SCROLLWIN_LINEUP;
			else
				newEvent.m_eventType = 
					wxEVT_SCROLLWIN_LINEDOWN;

			int times = abs(lines);
			for (; times > 0; times--)
				m_win->GetEventHandler()
					->ProcessEvent(newEvent);
		}
	}
}
#endif



BEGIN_EVENT_TABLE(CHMHtmlWindow, wxHtmlWindow)
#ifdef _ENABLE_COPY_AND_FIND
	EVT_MENU(ID_CopySel, CHMHtmlWindow::OnCopy)
	EVT_MENU(ID_PopupFind, CHMHtmlWindow::OnFind)
#endif
	EVT_MENU(ID_PopupForward, CHMHtmlWindow::OnForward)
	EVT_MENU(ID_PopupBack, CHMHtmlWindow::OnBack)
	EVT_MENU(ID_CopyLink, CHMHtmlWindow::OnCopyLink)
	EVT_RIGHT_DOWN(CHMHtmlWindow::OnRightClick)
#ifdef __WXMAC__
	EVT_MOUSEWHEEL(CHMHtmlWindow::HandleOnMouseWheel)
#endif
END_EVENT_TABLE()


