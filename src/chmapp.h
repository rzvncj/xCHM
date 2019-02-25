/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>

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
#include <config.h>
#endif

#include <wx/cmdline.h>
#include <wx/intl.h>
#include <wx/wx.h>

#ifdef WITH_LIBXMLRPC
#include <XmlRpc.h>
#endif

// Forward declaration.
class CHMFrame;

/*!
  \class wxApp
  \brief wxWidgets application class.
*/

//! This is the application class.
#ifdef WITH_LIBXMLRPC
class CHMApp : public wxApp, public XmlRpc::XmlRpcServerMethod {
#else
class CHMApp : public wxApp {
#endif

#ifdef WITH_LIBXMLRPC
public:
    //! Default constructor, also links the XMLRPC method
    CHMApp();
#endif

private:
    //! Our entry point into the application.
    bool OnInit() override;

#ifdef __WXMAC__
    //! Respond to Apple Event for opening a document
    void MacOpenFile(const wxString& filename) override;
#endif

#ifdef WITH_LIBXMLRPC
    //! Handles actual XMLRPC requests and parameter parsing.
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);

    //! Watches for XMLRPC requests
    void WatchForXMLRPC(wxTimerEvent& event);
#endif

    // Try to figure out the absolute file path of the executable.
    wxString getAppPath(const wxString& argv0, const wxString& cwd);

private:
    CHMFrame* _frame;
    wxLocale  _loc;

#ifdef WITH_LIBXMLRPC
    wxTimer _timer;
#endif
    wxCmdLineParser _cmdLP;
#ifdef WITH_LIBXMLRPC
    DECLARE_EVENT_TABLE()
#endif
};

#endif // __CHMAPP_H_
