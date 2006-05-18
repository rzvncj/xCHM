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


#include <string>


//! Fast index/contents file parser
class HHCParser {

public:
	//! Constructor
	HHCParser();

public:
	//! Parse a chunk of data.
	void parse(const char* chunk);

private:
	//! Handle a retrieved tag. I'm only interested in very few tags.
	void handleTag(const std::string& tag);

	//! Retrieve a parameter name.
	std::string getParameter(const char* input, const char* name,
				 bool lower = false);

private:
	int _level;
	bool _inquote;
	bool _intag;
	bool _inobject;
	std::string _tag;
	std::string _name;
	std::string _value;
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

