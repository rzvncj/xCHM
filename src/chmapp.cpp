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


#include <chmapp.h>
#include <chmframe.h>
#include <chmfshandler.h>
#include <chmfile.h>


bool CHMApp::OnInit()
{
	long xorig = 50, yorig = 50, width = 600, height = 450;
	long fontSize = CHM_DEFAULT_FONT_SIZE;
	wxString lastOpenedDir, normalFont, fixedFont;

	wxInitAllImageHandlers();
	wxFileSystem::AddHandler(new CHMFSHandler);

	wxConfig config("xchm");
	if(config.Read("/Position/xOrig", &xorig)) {
		config.Read("/Position/yOrig", &yorig);
		config.Read("/Position/width", &width);
		config.Read("/Position/height", &height);
		config.Read("/Paths/lastOpenedDir", &lastOpenedDir);
		config.Read("/Fonts/normalFontFace", &normalFont);
		config.Read("/Fonts/fixedFontFace", &fixedFont);
		config.Read("/Fonts/size", &fontSize);
	}
	
	CHMFrame *frame = new CHMFrame("xCHM v. " VERSION,
				       lastOpenedDir, wxPoint(xorig, yorig), 
				       wxSize(width, height), normalFont,
				       fixedFont, static_cast<int>(fontSize));

	frame->SetSizeHints(200, 200);
	frame->Show(TRUE);
	SetTopWindow(frame);

	if(argc > 1)
		frame->LoadCHM(argv[1]);
	
	return TRUE;
}



// Apparently this macro gets main() pumping.
IMPLEMENT_APP(CHMApp)


