/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>
  XML-RPC/Context ID code contributed by Eamon Millman / PCI Geomatics
  <millman@pcigeomatics.com>
  Mac OS patches contributed by Mojca Miklavec <mojca@macports.org>

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

#include <bitfiddle.inl>
#include <chmapp.h>
#include <chmfile.h>
#include <chmframe.h>
#include <chmfshandler.h>
#include <wx/config.h>
#include <wx/filefn.h>
#include <wx/fs_inet.h>
#include <wx/fs_mem.h>
#include <wx/image.h>

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef WITH_LIBXMLRPC

using namespace XmlRpc;

// The better way to use a singleton. Can't rely on the order of
// construction/initialization of global objects.
XmlRpcServer& getXmlRpcServer()
{
    static XmlRpcServer s;
    return s;
}

CHMApp::CHMApp() : wxApp(), XmlRpcServerMethod("xCHM", &getXmlRpcServer())
{
}

void CHMApp::execute(XmlRpcValue& params, XmlRpcValue& result)
{
    result = FALSE;
    if (params.size() > 0 && params[0].getType() == XmlRpcValue::TypeInt) {
        switch (int(params[0])) {
        case 0: // we want to close everything up!
            ExitMainLoop();
            result = TRUE;
            break;
        case 1:
            if (params.size() == 2 && params[1].getType() == XmlRpcValue::TypeString)
                result = _frame->LoadCHM(CURRENT_CHAR_STRING(std::string(params[1]).c_str()));

            if (params.size() == 3 && params[1].getType() == XmlRpcValue::TypeString
                && params[2].getType() == XmlRpcValue::TypeInt)
                result = _frame->LoadCHM(CURRENT_CHAR_STRING(std::string(params[1]).c_str()))
                    && _frame->LoadContextID(int(params[2]));
            break;
        case 2:
            if (params.size() == 2 && params[1].getType() == XmlRpcValue::TypeInt)
                result = _frame->LoadContextID(int(params[1]));
            break;
        }
    }
}
#endif

bool CHMApp::OnInit()
{
#if defined __WXMAC__ && WX_VERSION_MAJOR == 2
    ProcessSerialNumber PSN;
    GetCurrentProcess(&PSN);
    TransformProcessType(&PSN, kProcessTransformToForegroundApplication);
#endif

#ifdef WITH_LIBXMLRPC
    long port = -1;
#endif
    long     id = -1;
    wxString file;

    _cmdLP.SetCmdLine(argc, argv);

    _cmdLP.AddParam(wxT("file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    _cmdLP.AddOption(wxT("c"), wxT("contextid"), wxT("context-Id to open in file, requires that a file be specified"),
                     wxCMD_LINE_VAL_NUMBER);
#ifdef WITH_LIBXMLRPC
    _cmdLP.AddOption(wxT("x"), wxT("xmlrpc"), wxT("starts xCHM in XML-RPC server mode listening on port <num>"),
                     wxCMD_LINE_VAL_NUMBER);
#endif

    _cmdLP.AddSwitch(wxT("t"), wxT("notopics"), wxT("don't load the topics tree"));
    _cmdLP.AddSwitch(wxT("i"), wxT("noindex"), wxT("don't load the index"));
    _cmdLP.AddSwitch(wxT("h"), wxT("help"), wxT("displays this message."), wxCMD_LINE_OPTION_HELP);

    if (_cmdLP.Parse() != 0) // 0 means everything is ok
        return false;

    bool loadTopics = !_cmdLP.Found(wxT("notopics"));
    bool loadIndex  = !_cmdLP.Found(wxT("noindex"));

#ifdef WITH_LIBXMLRPC
    // catch the xmlrpc setup if desired
    _cmdLP.Found(wxT("xmlrpc"), &port);
#endif
    if (_cmdLP.GetParamCount() == 1) {
        file = _cmdLP.GetParam(0);
        _cmdLP.Found(wxT("contextid"), &id);

    } else if (_cmdLP.Found(wxT("contextid"))) {
        // can't use a context-ID without a file!
        _cmdLP.Usage();
        return false;
    }

    long     xorig = 50, yorig = 50, width = 600, height = 450;
    long     sashPos  = CONTENTS_MARGIN;
    long     fontSize = CHM_DEFAULT_FONT_SIZE;
    wxString lastOpenedDir, normalFont, fixedFont;

#if !defined(__WXMAC__) && !defined(__WXMSW__)
    _loc.Init();
    _loc.AddCatalog(wxT("xchm"));
#endif

    wxInitAllImageHandlers();
    wxFileSystem::AddHandler(new CHMFSHandler);
    wxFileSystem::AddHandler(new wxInternetFSHandler);
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    wxConfig config(wxT("xchm"));
    if (config.Read(wxT("/Position/xOrig"), &xorig)) {
        config.Read(wxT("/Position/yOrig"), &yorig);
        config.Read(wxT("/Position/width"), &width);
        config.Read(wxT("/Position/height"), &height);
        config.Read(wxT("/Paths/lastOpenedDir"), &lastOpenedDir);
        config.Read(wxT("/Fonts/normalFontFace"), &normalFont);
        config.Read(wxT("/Fonts/fixedFontFace"), &fixedFont);
        config.Read(wxT("/Fonts/size"), &fontSize);
        config.Read(wxT("/Sash/leftMargin"), &sashPos);
    }

    wxString fullAppPath;

    if (argc > 0)
        fullAppPath = getAppPath(argv[0], wxGetCwd());

    _frame = new CHMFrame(wxT("xCHM v. ") wxT(VERSION), lastOpenedDir, wxPoint(xorig, yorig), wxSize(width, height),
                          normalFont, fixedFont, static_cast<int>(fontSize), static_cast<int>(sashPos), fullAppPath,
                          loadTopics, loadIndex);

    _frame->SetSizeHints(200, 200);
    _frame->Show(TRUE);
    SetTopWindow(_frame);

#ifdef WITH_LIBXMLRPC
    if (port != -1) {
        _timer.SetOwner(this, TIMER_ID);
        _timer.Start(100);
        getXmlRpcServer().bindAndListen(port);
    }
#endif

    if (_cmdLP.GetParamCount() == 1) {
        _frame->LoadCHM(file);

        if (id != -1)
            _frame->LoadContextID(id);
    }

    return true;
}

wxString CHMApp::getAppPath(const wxString& argv0, const wxString& cwd)
{
    if (wxIsAbsolutePath(argv0))
        return argv0;

    wxString cwdtmp(cwd), apppath;

    if (cwdtmp.Last() != wxFILE_SEP_PATH)
        cwdtmp += wxFILE_SEP_PATH;

    apppath = cwdtmp + apppath;

    if (wxFileExists(apppath))
        return apppath;

    wxPathList pathList;
    pathList.AddEnvList(wxT("PATH"));
    apppath = pathList.FindAbsoluteValidPath(argv0);

    if (!apppath.IsEmpty())
        return wxPathOnly(apppath);

    return wxEmptyString;
}

#ifdef __WXMAC__
void CHMApp::MacOpenFile(const wxString& filename)
{
    _frame->LoadCHM(filename);
}
#endif

#ifdef WITH_LIBXMLRPC
void CHMApp::WatchForXMLRPC(wxTimerEvent&)
{
    getXmlRpcServer().work(0.0); // check for a XMLRPC message
}
#endif

#ifdef WITH_LIBXMLRPC
BEGIN_EVENT_TABLE(CHMApp, wxApp)
EVT_TIMER(TIMER_ID, CHMApp::WatchForXMLRPC)
END_EVENT_TABLE()
#endif

// Apparently this macro gets main() pumping.
IMPLEMENT_APP(CHMApp)
