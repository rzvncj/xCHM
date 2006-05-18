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

	if(!_inobject) {

		if(tagName == "ul") {
			++_level;

		} else if(tagName == "/ul") {
			if(_level > 0)
				--_level;

		} else if(tagName == "object") {
			
			_inobject = true;
		}
	
	} else {		
		if(tagName == "/object")
			_inobject = false;
	}


}


std::string HHCParser::getParameter(const char* input, const char* name)
{
	bool inquote = false;
	std::string tmpnm, tmpval;

	while(*input) {

		if(*input == '\"') {
			inquote = !inquote;
			++input;
			continue;
		}

		
	}
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


