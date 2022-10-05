/*
  Copyright (C) 2003 - 2022  Razvan Cojocaru <rzvncj@gmail.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.
*/

#ifndef __HHCPARSER_H_
#define __HHCPARSER_H_

#include <memory>
#include <string>
#include <wx/font.h>
#include <wx/treectrl.h>

// Forward declarations.
class CHMListCtrl;

//! Maximum number of tree levels.
constexpr size_t TREE_BUF_SIZE {128};

/*!
  \brief Objects of this class will be used as opaque data to be used with a tree item, so that when the user selects
  a tree item it will be easy to retrieve the filename associated with the item.
*/
struct URLTreeItem : public wxTreeItemData {

    //! Sets the data to str.
    explicit URLTreeItem(const wxString& str) : _url(str) {}

    //! Useful data.
    wxString _url;
};

//! Fast index/contents file parser
class HHCParser {

public:
    //! Constructor
    HHCParser(wxFontEncoding enc, wxTreeCtrl* tree, CHMListCtrl* list);

public:
    //! Parse a chunk of data.
    void parse(const char* chunk);

private:
    //! Handle a retrieved tag. I'm only interested in very few tags.
    void handleTag(const std::string& tag);

    //! Retrieve a parameter name.
    bool getParameters(const char* input, std::string& name, std::string& value);

    //! Add the information to the contents tree
    void addToTree(const wxString& name, const wxString& value);

    //! Add the information to the index list.
    void addToList(const wxString& name, const wxString& value);

    //! Replace special HTML strings with correct code.
    wxString replaceHTMLChars(const wxString& input);

    //! HTML code for given name (if available)
    unsigned getHTMLCode(const wxString& name);

public:
    //! Prevent copying, we have an unique_ptr<> member
    HHCParser(const HHCParser&) = delete;

    //! Prevent copying, we have an unique_ptr<> member
    HHCParser& operator=(const HHCParser&) = delete;

private:
    int                       _level {0};
    bool                      _inquote {false};
    bool                      _intag {false};
    bool                      _inobject {false};
    std::string               _tag;
    std::string               _name;
    std::string               _value;
    wxTreeCtrl*               _tree;
    CHMListCtrl*              _list;
    wxTreeItemId              _parents[TREE_BUF_SIZE] {};
    wxFontEncoding            _enc;
    int                       _counter {0};
    std::unique_ptr<wxCSConv> _cvPtr;
    bool                      _htmlChars {false};
};

#endif // __HHCPARSER_H_
