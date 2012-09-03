/*

  Copyright (C) 2003 - 2012  Razvan Cojocaru <rzvncj@gmail.com>
  ListDirty() patch contributed by Iulian Dragos
  <dragosiulian@users.sourceforge.net>
 
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


#ifndef __CHMLISTCTRL_H_
#define __CHMLISTCTRL_H_

#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/string.h>
#include <wx/dynarray.h>


// Forward declarations.
class CHMHtmlNotebook;


//! Item to store in the virtual list control
struct CHMListPairItem {
	//! Trivial constructor
	CHMListPairItem(const wxString& title, const wxString& url)
		: _title(title), _url(url) {}

	//! This will show up in the list.
	wxString _title;
	//! This is what the title points to.
	wxString _url;
};


//! Declare a wxWidgets sorted array
WX_DEFINE_SORTED_ARRAY(CHMListPairItem *, ItemPairArray);

//! Comparison function to use with the sorted array above.
int CompareItemPairs(CHMListPairItem *item1, CHMListPairItem *item2);


/*! 
  \class wxListCtrl
  \brief wxWidgets list control class.
*/


//! List control class meant to emulate the look and feel of a wxListBox.
class CHMListCtrl : public wxListCtrl {

public:
	/*!
	  \brief Initializes the custom list control.
	  \param parent The parent widget.
	  \param nbhtml Pointer to a CHMHtmlWindow that I'll associate with
	  this object so that selecting an item from the list will display
	  the corresponding page in the HTML window.
	  \param id Widget id.
	 */
	CHMListCtrl(wxWindow *parent, CHMHtmlNotebook *nbhtml,
		    wxWindowID id = -1);

	//! Cleanup.
	~CHMListCtrl();

public:
	//! Cleans up and removes all the list items.
	void Reset();

	/*!
	  \brief Adds a title:url pair to the list. The title is the part
	  that gets displayed, the url is tha page where the HTML window
	  should go when the item is being clicked.
	  \param title The title to add.
	  \param url The title's associated url.
	 */
	void AddPairItem(const wxString& title, const wxString& url);
	
	//! Loads the page that corresponds to the item currently selected.
	void LoadSelected();

	//! Should be called each time the list control's state changes.
	void UpdateUI();
  
	/*! 
	  \brief Finds the list item that is the best match.
	  \param title The string to match against.
	*/
	void FindBestMatch(const wxString& title);

protected:
	//! Gets called when the widget is being resized.
	void OnSize(wxSizeEvent& event);

	//! Gets called when an item needs to be displayed.
	wxString OnGetItemText(long item, long column) const;

private:
	//! Delete/empty the items in the item array.
	void ResetItems();
	
private:
	ItemPairArray _items;
	CHMHtmlNotebook *_nbhtml;
	int _currentSize;

private:
	DECLARE_EVENT_TABLE()
};


#endif // __CHMLISTCTRL_H_


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

