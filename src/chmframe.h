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
#	define VERSION "0.5"
#endif


// IDs for various widget events.
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


class CHMFrame : public wxFrame {
public:
	CHMFrame(const wxString& title, const wxString& booksDir,
		 const wxPoint& pos, const wxSize& size);
	~CHMFrame();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnHome(wxCommandEvent& event);	
	void OnHistoryForward(wxCommandEvent& event);	
	void OnHistoryBack(wxCommandEvent& event);	
	void OnShowContents(wxCommandEvent& event);	
	void OnPrint(wxCommandEvent& event);	
	void OnSelectionChanged(wxTreeEvent& event);
	void OnCloseWindow(wxCloseEvent& event);

	void LoadCHM(const wxString& archive);

private:
	wxMenuBar *CreateMenu();
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
