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


#ifndef __CONTENTTAGHANDLER_H_
#define __CONTENTTAGHANDLER_H_

#include <wx/html/htmlpars.h>
#include <wx/treectrl.h>


class ContentParser : public wxHtmlParser {
public:
	wxObject* GetProduct() { return NULL; }

protected:
	virtual void AddText(const wxChar* WXUNUSED(txt)) {}
};


struct URLTreeItem : public wxTreeItemData {
	URLTreeItem(const wxString& str) : _url(str) {}		
	wxString _url;
};


#define TREE_BUF_SIZE 128

class ContentTagHandler : public wxHtmlTagHandler {

public:
	ContentTagHandler(wxTreeCtrl* toBuild);

        wxString GetSupportedTags() { return wxT("UL,OBJECT,PARAM"); }
        bool HandleTag(const wxHtmlTag& tag);

private:
	wxTreeItemId _parents[TREE_BUF_SIZE];
	int _level;
	wxTreeCtrl* _treeCtrl;
	
	wxString _title;
	wxString _url;
};


#endif // __CONTENTTAGHANDLER_H_
