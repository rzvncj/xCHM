/*

  Copyright (C) 2003 - 2014  Razvan Cojocaru <rzvncj@gmail.com>

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


#ifndef __CHMAPP_H_
#define __CHMAPP_H_

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <wx/wx.h>
#include <wx/intl.h>
#include <wx/cmdline.h>

#ifdef WITH_LIBXMLRPC
#	define TIMER_ID	wxID_HIGHEST + 1
#	include <XmlRpc.h>
using namespace XmlRpc;
#endif


// Forward declaration.
class CHMFrame;


/*!
  \class wxApp
  \brief wxWidgets application class.
*/

//! This is the application class.
#ifdef WITH_LIBXMLRPC
class CHMApp : public wxApp, public XmlRpcServerMethod {
#else
class CHMApp : public wxApp {
#endif

#ifdef WITH_LIBXMLRPC
public:
	//! Default constructor, also links the XMLRPC method
	CHMApp();
#endif
	//! Our entry point into the application.
	virtual bool OnInit();

#ifdef __WXMAC__
	//! Respond to Apple Event for opening a document
	virtual void MacOpenFile(const wxString& filename);
#endif

protected:

#ifdef WITH_LIBXMLRPC
	//! Handles actual XMLRPC requests and parameter parsing.
	void execute(XmlRpcValue& params, XmlRpcValue& result);

	//! Watches for XMLRPC requests
	void WatchForXMLRPC( wxTimerEvent& event );
#endif

private:
	// Try to figure out the absolute file path of the executable.
	wxString getAppPath(const wxString& argv0, const wxString& cwd);

private:
	CHMFrame* _frame;
	wxLocale _loc;

#ifdef WITH_LIBXMLRPC
	wxTimer _timer;
#endif
	wxCmdLineParser _cmdLP;
#ifdef WITH_LIBXMLRPC
	DECLARE_EVENT_TABLE()
#endif
};

#endif // __CHMAPP_H_


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

