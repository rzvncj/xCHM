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


#ifndef __HHCPARSER_H_
#define __HHCPARSER_H_


#include <wx/font.h>
#include <wx/treectrl.h>
#include <string>


// Forward declarations.
class CHMListCtrl;


//! Maximum number of tree levels.
#define TREE_BUF_SIZE 128



/*! 
  \brief Objects of this class will be used as opaque data to be used with
  a tree item, so that when the user selects a tree item it will be easy
  to retrieve the filename associated with the item.
*/
struct URLTreeItem : public wxTreeItemData {

	//! Sets the data to str.
	URLTreeItem(const wxString& str) : _url(str) {}

	//! Useful data.
	wxString _url;
};


//! Fast index/contents file parser
class HHCParser {

public:
	//! Constructor
	HHCParser(wxFontEncoding enc, wxTreeCtrl *tree, CHMListCtrl *list);

public:
	//! Parse a chunk of data.
	void parse(const char* chunk);

private:
	//! Handle a retrieved tag. I'm only interested in very few tags.
	void handleTag(const std::string& tag);

	//! Retrieve a parameter name.
	bool getParameters(const char* input, std::string& name,
			   std::string& value);

	//! Add the information to the contents tree
	void addToTree(const wxString& name, const wxString& value);

	//! Add the information to the index list.
	void addToList(const wxString& name, const wxString& value);

	//! Replace special HTML strings with correct code.
	wxString replaceHTMLChars(const wxString& input);

	//! Convert to proper character encoding.
	wxString translateEncoding(const wxString& input);

	//! HTML code for given name (if available)
	unsigned getHTMLCode(const wxString& name);

	//! Get the char that code stands for
	wxChar charForCode(unsigned code);

private:
	int _level;
	bool _inquote;
	bool _intag;
	bool _inobject;
	std::string _tag;
	std::string _name;
	std::string _value;
	wxTreeCtrl *_tree;
	CHMListCtrl *_list;
	wxTreeItemId _parents[TREE_BUF_SIZE];
	wxFontEncoding _enc;
	int _counter;
	wxCSConv _cv;
	bool _htmlChars;
};


#endif // __HHCPARSER_H_


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

