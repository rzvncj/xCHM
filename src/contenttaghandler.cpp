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


#include <contenttaghandler.h>


ContentTagHandler::ContentTagHandler(wxTreeCtrl* toBuild)
	: _level(0), _treeCtrl(toBuild)
{
	if(!_treeCtrl)
		return;

	_parents[_level] = _treeCtrl->AddRoot(wxT("Topics"));
}


bool ContentTagHandler::HandleTag(const wxHtmlTag& tag)
{
	if(!_treeCtrl)
		return FALSE;

	if (tag.GetName() == wxT("UL"))
	{
		if(_level >= TREE_BUF_SIZE)
			return FALSE;
		
		++_level;
		ParseInner(tag);
		--_level;

		return TRUE;

	} else if (tag.GetName() == wxT("OBJECT")) {
        
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
		
		if (tag.GetParam(wxT("TYPE")) == wxT("text/sitemap")) {

			// handle broken <UL> tags that would
			// otherwise SEGFAULT by trying to access
			// _parents[-1].
			int parentIndex = _level ? _level - 1 : 0;

			_title = _url = wxEmptyString;
			ParseInner(tag);

			_parents[_level] = 
				_treeCtrl->AppendItem(_parents[parentIndex],
						      _title, -1, -1,
						      new URLTreeItem(_url));
			if(!_level)
				_parents[0] = _treeCtrl->GetRootItem();

			return TRUE;
		}
		
		return FALSE;
	} else { // "PARAM"

		if (tag.GetParam(wxT("NAME")) == wxT("Name"))
			_title = tag.GetParam(wxT("VALUE"));
		if (tag.GetParam(wxT("NAME")) == wxT("Local"))
			_url = tag.GetParam(wxT("VALUE"));

		return FALSE;
	}
}


