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
#include <hhcparser.h>
#include <chminputstream.h>
#include <chmframe.h>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/uri.h>




CHMHtmlWindow::CHMHtmlWindow(wxWindow *parent, wxTreeCtrl *tc, CHMFrame *frame)
	: wxHtmlWindow(parent, -1, wxDefaultPosition, wxSize(200,200)),
	  _tcl(tc), _syncTree(true), _found(false), _menu(NULL), 
	  _absolute(false), _frame(frame), _fdlg(NULL)
{
	_menu = new wxMenu;
	_menu->Append(ID_PopupForward, _("For&ward"));
	_menu->Append(ID_PopupBack, _("&Back"));
	_menu->Append(ID_CopyLink, _("Copy &link location"));

	_menu->AppendSeparator();
	_menu->Append(ID_CopySel, _("&Copy selection"));
	_menu->AppendSeparator();
	_menu->Append(ID_PopupFind, _("&Find in page.."));
}


CHMHtmlWindow::~CHMHtmlWindow()
{
	delete _menu;
	delete _fdlg;
}


bool CHMHtmlWindow::LoadPage(const wxString& location)
{
	wxLogNull log;
	wxString tmp = location;

	// Path is already absolute.
	if(_absolute) {
		FixPath(tmp, wxT("/"));
		_absolute = false;
	} else
		FixPath(tmp, _prefix);

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

	wxTreeItemIdValue cookie;
	wxTreeItemId child = _tcl->GetFirstChild(root, cookie);

	for(size_t i = 0; i < _tcl->GetChildrenCount(root, FALSE); ++i) {
		Sync(child, page);
		child = _tcl->GetNextChild(root, cookie);
	}
}


wxString CHMHtmlWindow::GetPrefix(const wxString& location) const
{
	return location.AfterLast(wxT(':')).AfterFirst(
		           wxT('/')).BeforeLast(wxT('/'));
}


bool CHMHtmlWindow::FixPath(wxString &location,
			    const wxString& prefix) const
{
	wxLogNull wln;

        if(!location.Left(5).CmpNoCase(wxT("http:")) ||
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

	bool full = location.StartsWith(wxT("file:"));
	bool repairWX = false;
	size_t len = 0;

	if(full) {
		wxString basepath = GetParser()->GetFS()->GetPath();
		len = basepath.Length();
		
		repairWX = location.StartsWith(basepath)
			&& len < location.Length()
			&& location[len] == wxT('/');
	}

	if(!repairWX && full && !location.Contains(wxT("./")))
		return false;
	
	wxString arch;
	wxString file;

	if(full)
		arch = location.BeforeFirst(wxT('#')).Mid(5);
	else
		arch = chmf->ArchiveName();			

	wxFileName awfn(arch);
	awfn.Normalize();

	if(full) {
		if(!repairWX)
			file = location.AfterFirst(wxT('#')).
				AfterFirst(wxT(':'));
		else 
			file = location.Mid(len);
	} else 
		file = location;

	if(!file.StartsWith(wxT("/"))) 
		file = wxString(wxT("/")) + prefix + wxT("/") + file;

	wxFileName fwfn(file, wxPATH_UNIX);
	fwfn.Normalize(wxPATH_NORM_DOTS);

	location = wxString(wxT("file:")) + awfn.GetFullPath() +
		wxString(wxT("#xchm:")) + fwfn.GetFullPath(wxPATH_UNIX);
	
	return true;
}


wxHtmlOpeningStatus CHMHtmlWindow::OnOpeningURL(wxHtmlURLType type,
						const wxString& url, 
						wxString *redirect) const
{
	if(type == wxHTML_URL_PAGE)
		return wxHTML_OPEN;

	wxString tmp = url;

	if(FixPath(tmp, _prefix)) {
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
			new wxTextDataObject(_link));
		wxTheClipboard->Close();
	}
}


void CHMHtmlWindow::OnRightClick(wxMouseEvent& event)
{
	if(IsSelectionEnabled())
		_menu->Enable(ID_CopySel, m_selection != NULL);

	_menu->Enable(ID_PopupForward, HistoryCanForward());
	_menu->Enable(ID_PopupBack, HistoryCanBack());
	_menu->Enable(ID_CopyLink, false);

        int x, y;
        CalcUnscrolledPosition(event.m_x, event.m_y, &x, &y);

	wxHtmlCell *cell = GetInternalRepresentation()->
		FindCellByPos(x, y);

	wxHtmlLinkInfo* linfo = NULL;

	if(cell)
		linfo = cell->GetLink();

	if(linfo) {
		_link = linfo->GetHref();
		_menu->Enable(ID_CopyLink, true);
	}

	PopupMenu(_menu, event.GetPosition());
}


void CHMHtmlWindow::OnChar(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_SPACE) {
		event.m_keyCode = WXK_PAGEDOWN;

	} else if(event.GetKeyCode() == WXK_BACK) {
		event.m_keyCode = WXK_PAGEUP;
	}

	event.Skip();
}


BEGIN_EVENT_TABLE(CHMHtmlWindow, wxHtmlWindow)
	EVT_MENU(ID_CopySel, CHMHtmlWindow::OnCopy)
	EVT_MENU(ID_PopupFind, CHMHtmlWindow::OnFind)
	EVT_MENU(ID_PopupForward, CHMHtmlWindow::OnForward)
	EVT_MENU(ID_PopupBack, CHMHtmlWindow::OnBack)
	EVT_MENU(ID_CopyLink, CHMHtmlWindow::OnCopyLink)
	EVT_CHAR(CHMHtmlWindow::OnChar)
	EVT_RIGHT_DOWN(CHMHtmlWindow::OnRightClick)
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


