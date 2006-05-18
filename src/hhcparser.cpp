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


#include <chmlistctrl.h>
#include <hhcparser.h>
#include <ctype.h>
#include <wx/wx.h>
#include <bitfiddle.inl>


#include <iostream>
using namespace std;


#define TIME_TO_YIELD	1024


HHCParser::HHCParser(wxFontEncoding enc, wxTreeCtrl *tree, CHMListCtrl *list)
	: _level(0), _inquote(false), _intag(false), _inobject(false),
	  _tree(tree), _list(list), _enc(enc), _counter(0)
{
	memset(_parents, 0, TREE_BUF_SIZE*sizeof(wxTreeItemId));
	
	if(_tree)
		_parents[_level] = _tree->AddRoot(_("Topics"));
}


void HHCParser::parse(const char* chunk)
{
	while(*chunk) {

		switch(*chunk) {
		case '\"':
			_inquote = !_inquote;
			break;

		case '<':
			if(!_inquote) {
				_intag = true;
				_tag = "";
				++chunk;
				continue;
			}

		case '>':
			if(!_inquote) {
				handleTag(_tag);
				_intag = false;
				++chunk;
				continue;
			}
		}
		
		if(_intag)
			_tag += *chunk;
		++chunk;
	}
}


void HHCParser::handleTag(const std::string& tag)
{
	if(tag.empty())
		return;	

	++_counter;

	if((_counter % TIME_TO_YIELD) == 0) {
		wxSafeYield();	
		_counter = 0;
	}

	size_t i;
	for(i = 0; i < tag.length(); ++i) {
		if(isspace(tag[i]))
			continue;
		else
			break;
	}

	if(i == tag.length())
		return;
	
	std::string tagName;

	for( ; i != tag.length(); ++i) {
		if(!isspace(tag[i]))
			tagName += tolower(tag[i]);
		else
			break;
	}

//	cerr << "TagName: " << tagName << endl;
	
	if(_inobject) {
		if(tagName == "/object") {
			_inobject = false;

			wxString name = makeWxString(_name);
			wxString value = CURRENT_CHAR_STRING(_value.c_str());

			addToTree(name, value);
			addToList(name, value);


		} else if(tagName == "param") {

			std::string name, value;
			getParameters(tag.c_str() + i, name, value);
			
			if(name == "name" && _name.empty()) 
				_name = value;
			else if(name == "local" && _value.empty())
				_value = value;
		}
	
	} else {		
		if(tagName == "ul") {
			++_level;
			
		} else if(tagName == "/ul") {
			if(_level > 0)
				--_level;

		} else if(tagName == "object") {
			_name = _value = "";
			_inobject = true;
		}
	}


}


void HHCParser::getParameters(const char* input, std::string& name,
			      std::string& value)
{
	bool lower = false;
	name = value = "";

	while(*input) {
		std::string tmpstr;
		
		while(*input && isspace(*input))
			++input;
			
		while(*input && !isspace(*input) && *input != '=') {
			tmpstr += tolower(*input);
			++input;
		}

		while(*input && isspace(*input))
			++input;

		if(*input) {
			if(*input != '=')
				return;
			else
				++input;
		}

		if(tmpstr == "name") {
			lower = true;
		} else if(tmpstr == "value") {
			lower = false;		
		} else {
			// now skip value.
			while(*input && isspace(*input))
				++input;
				
			if(*input && *input == '\"') {
				++input;
				while(*input && *input != '\"')
					++input;
				if(*input && *input == '\"')
					++input;
			} else {
				while(*input && !isspace(*input))
					++input;
			}
			continue;
		}

		while(*input && isspace(*input))
			++input;

		if(*input && *input == '\"') {
			++input;
			while(*input && *input != '\"') {
				if(lower)
					name += tolower(*input++);
				else 
					value += *input++;
			}
			
			if(*input && *input == '\"')
				++input;
		} else {
			while(*input && !isspace(*input))
				if(lower)
					name += tolower(*input++);
				else 
					value += *input++;
		}
	}
}


wxString HHCParser::makeWxString(const std::string& input)
{
#define BUF_SIZE 1024
	
	if(input.empty())
		return wxEmptyString;

	wxString s = CURRENT_CHAR_STRING(input.c_str());

#if wxUSE_UNICODE
	if(_enc != wxFONTENCODING_SYSTEM) {
		wxCSConv cv(_enc);
		
		s.Replace(wxT("&Dstrok;"), wxT("\320"), TRUE);

		wchar_t buf2[BUF_SIZE];
		size_t len = (s.length() < BUF_SIZE) ?
			s.length() : BUF_SIZE;

		size_t ret = cv.MB2WC(buf2, s.mb_str(), len);
		if(ret)
			return wxString(buf2, ret);
	} else
#endif
		return s;	
		
	return wxEmptyString;
}


void HHCParser::addToTree(const wxString& name, const wxString& value)
{
	if(!_tree)
		return;

	if(!_name.empty()) {
		
		int parentIndex = _level ? _level - 1 : 0;

		_parents[_level] = 
			_tree->AppendItem(_parents[parentIndex], name, 0, 0,
					  new URLTreeItem(value));
		if(!_level)
			_parents[0] = _tree->GetRootItem();
		else {
			_tree->SetItemImage(_parents[parentIndex], -1,
					    wxTreeItemIcon_Normal);
			
			_tree->SetItemImage(_parents[parentIndex], -1, 
					    wxTreeItemIcon_Selected);
		}
	}
}


void HHCParser::addToList(const wxString& name, const wxString& value)
{
	if(!_list)
		return;

	if(!name.IsEmpty() && !value.IsEmpty())
		_list->AddPairItem(name, value);
}



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


