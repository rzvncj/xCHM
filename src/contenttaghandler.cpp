/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
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


#include <chmlistctrl.h>
#include <contenttaghandler.h>
#include <wx/wx.h>
#include <wx/fontmap.h>
#include <string.h>
#include <wx/utils.h>


#define TIME_TO_YIELD	1024
#define BUF_SIZE	1024


ContentTagHandler::ContentTagHandler(wxFontEncoding enc, wxTreeCtrl* tree, 
				     CHMListCtrl *list)
	: _level(0), _treeCtrl(tree), _listCtrl(list), _enc(enc), _counter(0)
{
	if(!_treeCtrl && !_listCtrl)
		return;

	memset(_parents, 0, TREE_BUF_SIZE*sizeof(wxTreeItemId));
	
	if(_treeCtrl)
		_parents[_level] = _treeCtrl->AddRoot(_("Topics"));
}


bool ContentTagHandler::HandleTag(const wxHtmlTag& tag)
{
	if(!_treeCtrl && !_listCtrl)
		return FALSE;

	++_counter;

	if((_counter % TIME_TO_YIELD) == 0) {
		wxSafeYield();	
		_counter = 0;
	}

	if (tag.GetName() == wxT("UL"))
	{
		if(_treeCtrl && _level >= TREE_BUF_SIZE)
			return FALSE;

		bool inc = _level >= 0 
			&& (long)_parents[_level] != 0;
		
		if(inc)
			++_level;
		ParseInner(tag);
		
		if(inc)
			--_level;

		return TRUE;

	} else if(!tag.GetName().CmpNoCase(wxT("OBJECT"))) {
        
		/* 
		   Valid HHW's file may contain only two object tags: 
 
		   <OBJECT type="text/site properties"> 
		   <param name="ImageType" value="Folder"> 
		   </OBJECT> 
 
		   or 
 
		   <OBJECT type="text/sitemap"> 
		   <param name="Name" value="main page"> 
		   <param name="Local" value="another.htm"> 
		   </OBJECT> 
 
		   We're interested in the latter.
		*/
		
		if(!tag.GetParam(wxT("TYPE")).CmpNoCase(wxT("text/sitemap"))) {

			// handle broken <UL> tags that would
			// otherwise SEGFAULT by trying to access
			// _parents[-1].
			int parentIndex = _level ? _level - 1 : 0;

			_title = _url = wxEmptyString;
			ParseInner(tag);

			if(_treeCtrl && !_title.IsEmpty()) {

				_parents[_level] = 
					_treeCtrl->AppendItem(
						_parents[parentIndex],
						_title, 0, 0,
						new URLTreeItem(_url));
				if(!_level)
					_parents[0] = _treeCtrl->GetRootItem();
				else {
					_treeCtrl->SetItemImage(
						_parents[parentIndex], 
						-1, wxTreeItemIcon_Normal);

					_treeCtrl->SetItemImage(
						_parents[parentIndex], 
						-1, wxTreeItemIcon_Selected);
				}
			}

			return TRUE;
		}
		
		return FALSE;
	} else { // "PARAM"

		if(!tag.GetParam(wxT("NAME")).CmpNoCase(wxT("Name"))) {

			if(_title.IsEmpty()) {
#ifdef wxUSE_UNICODE
				if(_enc != wxFONTENCODING_SYSTEM) {
					wxCSConv cv(_enc);

					const wxString s = 
						tag.GetParam(wxT("VALUE"));

					wchar_t buf2[BUF_SIZE];
					size_t len =
						(s.length() < BUF_SIZE) ?
						s.length() : BUF_SIZE;

					size_t ret = cv.MB2WC(buf2, s.mb_str(),
							      len);
					if(ret)
						_title = wxString(buf2, ret);
				} else 
#endif
					_title = tag.GetParam(wxT("VALUE"));
			}
		}

		if(!tag.GetParam(wxT("NAME")).CmpNoCase(wxT("Local"))) {
			_url = tag.GetParam(wxT("VALUE"));
		}

		if(_listCtrl && !_url.IsEmpty() && !_title.IsEmpty()) {
			_listCtrl->AddPairItem(_title, _url);
			_title = _url = wxEmptyString;
		}

		return FALSE;
	}
}


