/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#ifndef __CHMLISTCTRL_H_
#define __CHMLISTCTRL_H_

#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/string.h>


// Forward declarations.
class CHMHtmlWindow;


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
	  \param html Pointer to a CHMHtmlWindow that I'll associate with
	  this object so that selecting an item from the list will display
	  the corresponding page in the HTML window.
	  \param id Widget id.
	 */
	CHMListCtrl(wxWindow *parent, CHMHtmlWindow *html,
		    wxWindowID id = -1);

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
	  Called by ContentHandler to allow the list to perform (expensive)
	  size computation.
	*/
	void ListDirty();

	/*! 
	  \brief Finds the list item that is the best match.
	  \param title The string to match against.
	*/
	void FindBestMatch(const wxString& title);

protected:
	//! Gets called when the widget is being resized.
	void OnSize(wxSizeEvent& event);
	
private:
	wxArrayString _urls;
	CHMHtmlWindow *_html;
	int _currentSize;

private:
	DECLARE_EVENT_TABLE()
};


#endif // __CHMLISTCTRL_H_

