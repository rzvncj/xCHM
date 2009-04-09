/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
  Tabbed browsing support developed by Cedric Boudinet <bouced@gmx.fr>
  (this file originally written by Cedric Boudinet)
 
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


#include <chmframe.h>
#include <chmhtmlwindow.h>
#include <chmhtmlnotebook.h>


CHMHtmlNotebook::CHMHtmlNotebook(wxWindow *parent, wxTreeCtrl *tc, 
				 CHMFrame* frame)
	: wxAuiNotebook(parent, -1, wxDefaultPosition, wxDefaultSize,
			wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_FIXED_WIDTH),
	  _tcl(tc), _frame(frame)
{
	wxAcceleratorEntry entries[2];  
	entries[0].Set(wxACCEL_CTRL,   WXK_PRIOR,     ID_PriorPage);
	entries[1].Set(wxACCEL_CTRL,   WXK_NEXT,      ID_NextPage);

	wxAcceleratorTable accel(2, entries);
	this->SetAcceleratorTable(accel);
	SetTabCtrlHeight(0);
	AddHtmlView(wxEmptyString, wxT("memory:about.html"));
}


CHMHtmlWindow* CHMHtmlNotebook::CreateView()
{
	CHMHtmlWindow * htmlWin = new CHMHtmlWindow(this, _tcl, _frame);
	htmlWin->SetRelatedFrame(_frame, wxT("xCHM v. " VERSION));
	htmlWin->SetRelatedStatusBar(0);
	htmlWin->SetFonts(_fonts_normal_face, _fonts_fixed_face, _fonts_sizes);

	AddPage(htmlWin, _("(Empty page)"));
	SetSelection(GetPageCount() - 1);
	return htmlWin;
}


void CHMHtmlNotebook::AddHtmlView(const wxString& path,
				  const wxString& link)
{
	CHMHtmlWindow* htmlWin = CreateView();
	
	if(htmlWin) {
		htmlWin->GetParser()->GetFS()->ChangePathTo(path);
		htmlWin->LoadPage(link);
	}
}


bool CHMHtmlNotebook::LoadPageInCurrentView(const wxString& location)
{
	return GetCurrentPage()->LoadPage(location);
}


// TODO: this is a misleading named function with side effects. It's a no-no.
CHMHtmlWindow* CHMHtmlNotebook::GetCurrentPage()
{
	int selection = GetSelection();

	if(selection == wxNOT_FOUND)
		return CreateView();

	return dynamic_cast<CHMHtmlWindow *>(
		wxAuiNotebook::GetPage(selection));
}


void CHMHtmlNotebook::OnGoToNextPage(wxCommandEvent&)
{
	int selection = GetSelection();

	if(selection >= static_cast<int>(GetPageCount() - 1))
		return;

	SetSelection(selection + 1);
}


void CHMHtmlNotebook::OnGoToPriorPage(wxCommandEvent&)
{
	int selection = GetSelection();

	if(selection <= 0)
		return;

	SetSelection(selection - 1);
}


void CHMHtmlNotebook::OnCloseTab(wxCommandEvent& WXUNUSED(event))
{
	DeletePage(GetSelection());

	if(GetPageCount() <= 1)
		SetTabCtrlHeight(0);
}


void CHMHtmlNotebook::OnNewTab(wxCommandEvent& WXUNUSED(event))
{
	AddHtmlView(wxEmptyString, wxEmptyString);
}


void CHMHtmlNotebook::OnChildrenTitleChanged(const wxString& title)
{
	// We assume the change occured in the currently displayed page
	// TODO: detect in which page the change occured.
	SetPageText(GetSelection(), title);
}


void CHMHtmlNotebook::CloseAllPagesExceptFirst()
{
	SetSelection(0);

	while(GetPageCount() > 1)
		DeletePage(1);

	SetTabCtrlHeight(0);
}


void CHMHtmlNotebook::SetChildrenFonts(const wxString& normal_face,
				       const wxString& fixed_face,
				       const int *sizes)
{
	_fonts_normal_face = normal_face;
	_fonts_fixed_face = fixed_face;

	for(int i = 0; i < 7; ++i)
		_fonts_sizes[i] = sizes[i];

	size_t nPageCount = GetPageCount();

	for(size_t nPage = 0; nPage < nPageCount; ++nPage) {
		CHMHtmlWindow* chw = 
			dynamic_cast<CHMHtmlWindow *>(GetPage(nPage));

		if(chw)
			chw->SetFonts(normal_face, fixed_face, sizes);
	}
}


bool CHMHtmlNotebook::AddPage(wxWindow* page, const wxString& title)
{
	if(!page)
		return false;

	bool st = wxAuiNotebook::AddPage(page, title);

	if(GetPageCount() == 2)
		SetTabCtrlHeight(-1);

	return st;
}


void CHMHtmlNotebook::OnPageChanged(wxAuiNotebookEvent&)
{
	if(GetPageCount() == 1)
		SetTabCtrlHeight(0);
}


BEGIN_EVENT_TABLE(CHMHtmlNotebook, wxAuiNotebook)
	EVT_MENU(ID_PriorPage, CHMHtmlNotebook::OnGoToPriorPage)
	EVT_MENU(ID_NextPage, CHMHtmlNotebook::OnGoToNextPage)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, CHMHtmlNotebook::OnPageChanged)
END_EVENT_TABLE()


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


