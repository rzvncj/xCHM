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
#include <wx/font.h>
#include <wx/string.h>


class CHMListCtrl;


/*!
  \class wxHtmlParser
  \brief wxWidgets HTML parser class.
*/


//! Useful only as a dummy. The real work is done by ContentTagHandler.
class ContentParser : public wxHtmlParser {
public:
	//! Dummy implementation. Returns NULL.
	wxObject* GetProduct() { return NULL; }

protected:
	//! Dummy implementation. Does nothing.
	virtual void AddText(const wxChar* WXUNUSED(txt)) {}
};


/*!
  \class wxTreeItemData
  \brief wxWidgets generic data type to associate with a wxTreeCtrl item.
*/


/*! 
  \brief Objects of this class will be used as opaque data to be used with
  a tree item, so that when the user selects a tree item it will be easy
  to retrieve the filename associated with the item.
*/
/*
struct URLTreeItem : public wxTreeItemData {

	//! Sets the data to str.
	URLTreeItem(const wxString& str) : _url(str) {}

	//! Useful data.
	wxString _url;
};
*/


/*!
  \class wxHtmlTagHandler
  \brief wxWidgets HTML tag handler class.
*/


//! Maximum number of tree levels.
#define TREE_BUF_SIZE 128


//! The busiest class in extracting the contents tree by parsing an index file.
class ContentTagHandler : public wxHtmlTagHandler {

public:
	/*!
	  \brief Constructs the tag handler.
	  \param enc Encoding to use. Only useful for Unicode builds.
	  \param tree Tree control to fill. The control must be
	  empty. Used for the content tree.
	  \param list Alternatively (or simultaneously), fill this custom
	  list control (used for the index).
	 */
	ContentTagHandler(wxFontEncoding enc, wxTreeCtrl* tree = NULL, 
			  CHMListCtrl *list = NULL);

	//! What tags are we interested in?
        wxString GetSupportedTags() { return wxT("UL,OBJECT,PARAM"); }

	/*!
	  \brief Does the bulk of the work, constructing the tree.
	  \param tag The current tag to handle.
	  \return true if we've parsed inner tags also, false otherwise.
	 */
        bool HandleTag(const wxHtmlTag& tag);

private:
	wxTreeItemId _parents[TREE_BUF_SIZE];
	int _level;
	wxTreeCtrl* _treeCtrl;
	CHMListCtrl* _listCtrl;
	
	wxString _title;
	wxString _url;

	wxFontEncoding _enc;
//	bool _useEnc;
	int _counter;
};


#endif // __CONTENTTAGHANDLER_H_


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
