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


#ifndef __CHMFRAME_HPP_
#define __CHMFRAME_HPP_

#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/splitter.h>


#ifdef HAVE_CONFIG_H
#	include <config.h>
#else
	// this should never happen
#	define VERSION "unknown"
#endif


//! IDs for various widget events.
enum
{
    ID_Quit = 1,
    ID_About,
    ID_Open,
    ID_Print,
    ID_Home,
    ID_Forward,
    ID_Back,
    ID_Contents,
    ID_TreeCtrl = 1000,
};


/*! 
  \brief The frame, owner of the wxHtmlWindow, contents tree control and
  all the other nifty stuff. 
*/
class CHMFrame : public wxFrame {
public:

	/*!
	  \brief Brings the frame into existence.
	  \param title The text that shows up on the titlebar.
	  \param booksDir Where to go when you click Open.. on the
	  toolbar. This is used to remember the last directory where
	  a book was sucessfully opened. For the current working
	  directory just pass the empty string.
	  \param pos The upper left corner of the frame.
	  \param size The size of the frame.
	*/
	CHMFrame(const wxString& title, const wxString& booksDir,
		 const wxPoint& pos, const wxSize& size);

	//! Cleans up.
	~CHMFrame();

	//! Called when the user closes the window.
	void OnQuit(wxCommandEvent& event);

	//! Called when the user clicks on About.
	void OnAbout(wxCommandEvent& event);

	//! Called when the user wants to open a file.
	void OnOpen(wxCommandEvent& event);

	//! Called when the user wants to see the default page.
	void OnHome(wxCommandEvent& event);

	//! Called when the user wants to go forward in the history.
	void OnHistoryForward(wxCommandEvent& event);

	//! Called when the user wants to go back in the history.
	void OnHistoryBack(wxCommandEvent& event);	

	/*! 
	  \brief Called when the user wants to either disable or enable
	  the contents tree panel on the left.
	*/
	void OnShowContents(wxCommandEvent& event);	

	//! Called when the user wants to print the displayed page.
	void OnPrint(wxCommandEvent& event);

	//! Called when an item in the contents tree is clicked.
	void OnSelectionChanged(wxTreeEvent& event);

	//! Cleanup code. This saves the window position and last open dir.
	void OnCloseWindow(wxCloseEvent& event);

	/*!
	  \brief Attempts to load a .chm file and display it's home page,
	  and if available, the contents tree. Otherwise an error message
	  is issued.
	  \param archive The .chm file name on disk.
	 */
	void LoadCHM(const wxString& archive);
	
private:
	//! Helper. Creates the menu.
	wxMenuBar *CreateMenu();

	//! Helper. Initializes the frame toolbar.
	bool InitToolBar(wxToolBar *toolbar);

private:
	wxHtmlWindow* _html;
	wxTreeCtrl* _tcl;
	wxSplitterWindow* _sw;
	wxMenu* _menuFile;
	wxToolBar* _tb;
	wxHtmlEasyPrinting *_ep;
	wxString _openPath;

private:
	DECLARE_EVENT_TABLE()	
};


#endif // __CHMFRAME_HPP_