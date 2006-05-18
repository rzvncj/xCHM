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


#include <hhcparser.h>
#include <ctype.h>


#include <iostream>
using namespace std;


HHCParser::HHCParser()
	: _level(0), _inquote(false), _intag(false), _inobject(false)
{}


void HHCParser::parse(const char* chunk)
{
	while(*chunk) {

		switch(*chunk) {
		case '\"':
			_inquote = !_inquote;
			++chunk;
			continue;

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

	cerr << "TagName: " << tagName << endl;
	
	if(_inobject) {
		if(tagName == "/object") {
			_inobject = false;

		} else if(tagName == "param") {

			std::string name = getParameter(tag.c_str() + i, 
							"name", true);

			cerr << "::::name::::" << name << endl;
			if(name == "name") {
				if(_name.empty())
					_name = getParameter(tag.c_str() + i, 
							     "value", false);
				cerr << "_name: " << _name << endl;
			} else if(name == "local") {
				if(_value.empty())
					_value = getParameter(tag.c_str() + i, 
							      "value", false);
				cerr << "_value: " << _value << endl;
			}
		}
	
	} else {		
		if(tagName == "ul") {
			++_level;
			
		} else if(tagName == "/ul") {
			if(_level > 0)
				--_level;

		} else if(tagName == "object") {
			_name = _value = "";
			if(getParameter(tag.c_str() + i, "type") 
			   == "text/sitemap")
				_inobject = true;
		}
	}


}


std::string HHCParser::getParameter(const char* input, const char* name,
				    bool lower)
{
	cerr << "Input: " << input << endl;

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

		if(*input && *input == '=')
			++input;

		if(tmpstr != name)
			continue;

		while(*input && isspace(*input))
			++input;

		tmpstr = "";

		if(*input && *input == '\"') {
			++input;
			while(*input && *input != '\"') {
				if(lower)
					tmpstr += tolower(*input++);
				else 
					tmpstr += *input++;
			}
		} else {
			while(*input && !isspace(*input))
				if(lower)
					tmpstr += tolower(*input++);
				else 
					tmpstr += *input++;
		}

		return tmpstr;
	}

	return "";
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


