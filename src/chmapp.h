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


#ifndef __CHMAPP_H_
#define __CHMAPP_H_

#include <wx/wx.h>
#include <wx/intl.h>


// Forward declaration.
class CHMFrame;


/*! 
  \class wxApp
  \brief wxWidgets application class.
*/

//! This is the application class.
class CHMApp : public wxApp {

	//! Our entry point into the application.
	virtual bool OnInit();

#ifdef __WXMAC__
	//! Respond to Apple Event for opening a document
	virtual void MacOpenFile(const wxString& filename);
#endif

private:
	CHMFrame* _frame;
	wxLocale _loc;
};


#endif // __CHMAPP_H_
