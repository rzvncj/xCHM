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


#include <chmhtmlwindow.h>
#include <contenttaghandler.h>
#include <chminputstream.h>


CHMHtmlWindow::CHMHtmlWindow(wxWindow *parent, wxTreeCtrl *tc)
	: wxHtmlWindow(parent, -1, wxDefaultPosition, wxSize(200,200)),
	  _tcl(tc), _syncTree(true), _found(false)
{}


bool CHMHtmlWindow::LoadPage(const wxString& location)
{
	wxString tmp = location;

	// handle relative paths
	if(tmp.StartsWith(wxT(".."))) {
		CHMFile *chmf = CHMInputStream::GetCache();

		if(chmf)
			tmp.Replace(wxT(".."), wxString(wxT("file:")) + 
				    chmf->ArchiveName() + wxT("#chm:") +
				    GetPrefix().BeforeLast(wxT('/')));
	
	} else if(tmp.StartsWith(wxT("javascript:fullSize"))) {
		tmp = tmp.AfterFirst(wxT('\'')).BeforeLast(wxT('\''));
	}

	if(_syncTree && 
	   // We should be looking for a valid page, not / (home).
	   !location.AfterLast(wxT('/')).IsEmpty() && 
	   _tcl->GetCount() > 1) {

		// Sync will call SelectItem() on the tree item
		// if it finds one, and that in turn will call
		// LoadPage() with _syncTree set to false.
		Sync(_tcl->GetRootItem(), tmp);

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

	wxString url, tmp;

	if(page.StartsWith(wxT("/")) || page.StartsWith(wxT("file:")))
		tmp = page.AfterLast(wxT(':')).AfterFirst(
			wxT('/')).BeforeFirst(wxT('#')).Lower();
	else
		tmp = page.Lower();

	if(data)
		url = (data->_url).BeforeFirst(wxT('#')).Lower();

	if(data && (url == tmp || url == GetPrefix() + wxT("/") + tmp)) {
		// Order counts!
		_found = true;
		_tcl->SelectItem(root);
		return;
	}

	long cookie;
	wxTreeItemId child = _tcl->GetFirstChild(root, cookie);

	for(size_t i = 0; i < _tcl->GetChildrenCount(root, FALSE); ++i) {
		Sync(child, page);
		child = _tcl->GetNextChild(root, cookie);
	}
}


inline 
wxString CHMHtmlWindow::GetPrefix()
{
	return GetOpenedPage().AfterLast(wxT(':')).AfterFirst(
		wxT('/')).BeforeLast(wxT('/')).Lower();
}

