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

#include <chmsearchpanel.h>
#include <contenttaghandler.h>
#include <chminputstream.h>
#include <wx/sizer.h>
#include <wx/utils.h>
#include <wx/regex.h>


CHMSearchPanel::CHMSearchPanel(wxWindow *parent, wxTreeCtrl *topics,
			       wxHtmlWindow *html)
	: wxPanel(parent), _tcl(topics), _text(NULL), _case(NULL),
	  _whole(NULL), _search(NULL), _results(NULL), _html(html)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetAutoLayout(TRUE);
        SetSizer(sizer);

	_text = new wxTextCtrl(this, ID_SearchText, wxEmptyString, 
			       wxDefaultPosition, wxDefaultSize, 
			       wxTE_PROCESS_ENTER);

	_case = new wxCheckBox(this, -1, wxT("Case sensitive"));
	_whole = new wxCheckBox(this, -1, wxT("Whole words only"));	
	_search = new wxButton(this, ID_SearchButton, wxT("Search"));

#if wxUSE_TOOLTIPS
	_search->SetToolTip(wxT("Search contents for all occurences of"
				" the specified text."));
#endif

	_results = new wxListBox(this, ID_Results, 
				 wxDefaultPosition, wxDefaultSize, 
				 0, NULL, wxLB_SINGLE);

        sizer->Add(_text, 0, wxEXPAND | wxALL, 10);
        sizer->Add(_case, 0, wxLEFT | wxRIGHT, 10);
        sizer->Add(_whole, 0, wxLEFT | wxRIGHT, 10);
	sizer->Add(_search, 0, wxALL, 10);
        sizer->Add(_results, 1, wxALL | wxEXPAND, 2);
}


void CHMSearchPanel::OnSearch(wxCommandEvent& WXUNUSED(event))
{
	wxBusyCursor bcr;
	wxString sr = _text->GetLineText(0);

	if (sr.IsEmpty() || _tcl->GetCount() <= 1)
		return;

	_results->Clear();

	PopulateList(_tcl->GetRootItem(), sr, _case->IsChecked(),
		     _whole->IsChecked());
}


void CHMSearchPanel::PopulateList(wxTreeItemId root, wxString& text,
				  bool caseSensitive, bool wholeWords)
{
	static CHMFile *chmf = CHMInputStream::GetCache();

	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(root));

	if(data && (!data->_url.IsEmpty())) {

		wxString tmp = wxString(wxT("file:")) + 
			chmf->ArchiveName() + wxT("#chm:/") + data->_url;

		if(SearchFile(tmp, text,
			      caseSensitive, wholeWords)) {
			_results->Append(_tcl->GetItemText(root),
					 new wxString(tmp));
		}
	}
	

	long cookie;
	wxTreeItemId child = _tcl->GetFirstChild(root, cookie);

	for(size_t i = 0; i < _tcl->GetChildrenCount(root, FALSE); ++i) {
		PopulateList(child, text, caseSensitive, wholeWords);
		child = _tcl->GetNextChild(root, cookie);
	}
}


static inline bool WHITESPACE(wxChar c)
{
	return c == _T(' ') || c == _T('\n') || c == _T('\r') || c == _T('\t');
}


bool CHMSearchPanel::SearchFile(const wxString& filename, 
				wxString& text,
				bool caseSensitive, bool wholeWords)
{
	size_t i, j, wrd = text.Length();

	bool found = FALSE;

        wxFileSystem fsys;
        wxFSFile *file = fsys.OpenFile(filename);

        if (file == NULL)
            return FALSE;

	wxHtmlFilterHTML filter;
	wxString tmp = filter.ReadFile(*file);
	int lng = tmp.length();

	if (!caseSensitive) {
		tmp.MakeLower();
		text.MakeLower();
	}

	const wxChar *buf = tmp.c_str();
	bool inTag = false;

	if(wholeWords) {
		for(i = 0; i < lng - wrd; ++i) {
			
			if(buf[i] == wxT('<'))
				inTag = true;
			else if(buf[i] == wxT('>'))
				inTag = false;

			if(WHITESPACE(buf[i]) || inTag) 
				continue;
			 			
			j = 0;
			while ((j < wrd) && (buf[i + j] == text[j])) 
				++j;
			if (j == wrd && WHITESPACE(buf[i + j])) { 
				found = TRUE; 
				break; 
			}
		}
	} else {
		for (i = 0; i < lng - wrd; ++i) {

  			if(buf[i] == wxT('<'))
				inTag = true;
			else if(buf[i] == wxT('>'))
				inTag = false;

			if(inTag)
				continue;

			j = 0;
			while ((j < wrd) && (buf[i + j] == text[j])) 
				++j;
			if (j == wrd) { 
				found = TRUE; 
				break; 
			}
		}
	}

	return found;
}


void CHMSearchPanel::OnSearchSel(wxCommandEvent& WXUNUSED(event))
{
	wxString *userData = reinterpret_cast<wxString *>(
		_results->GetClientData(_results->GetSelection()));

	_html->LoadPage(*userData);
}


void CHMSearchPanel::Reset()
{
	_text->Clear();
	_case->SetValue(FALSE);
	_whole->SetValue(FALSE);
	_results->Clear();
}



BEGIN_EVENT_TABLE(CHMSearchPanel, wxPanel)
    EVT_LISTBOX(ID_Results, CHMSearchPanel::OnSearchSel)
    EVT_BUTTON(ID_SearchButton, CHMSearchPanel::OnSearch)
    EVT_TEXT_ENTER(ID_SearchText, CHMSearchPanel::OnSearch)
END_EVENT_TABLE()


