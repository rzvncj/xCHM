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
#include <wx/config.h>


CHMSearchPanel::CHMSearchPanel(wxWindow *parent, wxTreeCtrl *topics,
			       wxHtmlWindow *html)
	: wxPanel(parent), _tcl(topics), _text(NULL), _case(NULL),
	  _whole(NULL), _titles(NULL), _search(NULL), _results(NULL), 
	  _html(html)
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetAutoLayout(TRUE);
        SetSizer(sizer);

	_text = new wxTextCtrl(this, ID_SearchText, wxEmptyString, 
			       wxDefaultPosition, wxDefaultSize, 
			       wxTE_PROCESS_ENTER);

	_case = new wxCheckBox(this, -1, wxT("Case sensitive"));
	_whole = new wxCheckBox(this, -1, wxT("Whole words only"));	
	_titles = new wxCheckBox(this, -1, wxT("Search titles only"));	
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
        sizer->Add(_titles, 0, wxLEFT | wxRIGHT, 10);
	sizer->Add(_search, 0, wxALL, 10);
        sizer->Add(_results, 1, wxALL | wxEXPAND, 2);

	GetConfig();
}


CHMSearchPanel::~CHMSearchPanel()
{
	SetConfig();
}


void CHMSearchPanel::OnSearch(wxCommandEvent& WXUNUSED(event))
{
	wxBusyCursor bcr;
	wxString sr = _text->GetLineText(0);

	if (sr.IsEmpty() || _tcl->GetCount() <= 1)
		return;

	_results->Clear();

	PopulateList(_tcl->GetRootItem(), sr, _case->IsChecked(),
		     _whole->IsChecked(), _titles->IsChecked());
}


void CHMSearchPanel::PopulateList(wxTreeItemId root, wxString& text,
				  bool caseSensitive, bool wholeWords,
				  bool titlesOnly)
{
	static CHMFile *chmf = CHMInputStream::GetCache();

	if(!chmf)
		return;

	URLTreeItem *data = reinterpret_cast<URLTreeItem *>(
		_tcl->GetItemData(root));

	if(data && (!data->_url.IsEmpty())) {

		wxString url = wxString(wxT("file:")) + 
			chmf->ArchiveName() + wxT("#chm:/") + data->_url;
		wxString title = _tcl->GetItemText(root);

		if(!titlesOnly) {
			if(FileSearch(url, text, caseSensitive, wholeWords))
				_results->Append(title, new wxString(url));
		} else {
			if(TitleSearch(title, text, caseSensitive, wholeWords))
				_results->Append(title, new wxString(url));
		}
	}	

	long cookie;
	wxTreeItemId child = _tcl->GetFirstChild(root, cookie);

	for(size_t i = 0; i < _tcl->GetChildrenCount(root, FALSE); ++i) {
		PopulateList(child, text, caseSensitive, wholeWords,
			     titlesOnly);
		child = _tcl->GetNextChild(root, cookie);
	}
}


static inline bool WHITESPACE(wxChar c)
{
	return c == wxT(' ') || c == wxT('\n') || c == wxT('\r') 
		|| c == wxT('\t') || c == wxT('<') || c == wxT('>');
}


static inline bool HTML_WHITESPACE(wxChar c)
{
	return WHITESPACE(c) || c == wxT('<') || c == wxT('>');
}


// The following two functions should be changed with faster ones.

bool CHMSearchPanel::FileSearch(const wxString& filename, wxString& text,
				bool caseSensitive, bool wholeWords)
{
	int i, j, wrd = text.Length();
	bool found = false;

	wxFileSystem fsys;
	wxFSFile *file = fsys.OpenFile(filename);

	if (file == NULL)
		return FALSE;

	wxHtmlFilterHTML filter;
	wxString tmp = filter.ReadFile(*file);

	int lng = tmp.Length();

	if (!caseSensitive) {
		tmp.MakeLower();
		text.MakeLower();
	}

	const wxChar *buf1 = tmp.c_str(), *buf2 = text.c_str();
	bool inTag = false;

	if(wholeWords) {
		for(i = 0; i < lng - wrd + 1; ++i) {

			if(buf1[i] == wxT('<'))
				inTag = true;
			else if(buf1[i] == wxT('>'))
				inTag = false;

			if(HTML_WHITESPACE(buf1[i]) || inTag) 
				continue;
			 			
			j = 0;
			while(buf1[i + j] == buf2[j] && j < wrd) 
				++j;

			if (j == wrd && 
			    (HTML_WHITESPACE(buf1[i + j]) || i+j == lng)) 
				if(i == 0 || HTML_WHITESPACE(buf1[i - 1])) {
					found = TRUE; 
					break; 
				}
		}
	} else {
		for (i = 0; i < lng - wrd + 1; ++i) {

  			if(buf1[i] == wxT('<'))
				inTag = true;
			else if(buf1[i] == wxT('>')) {
				inTag = false;
				continue;
			}

			if(inTag)
				continue;

			j = 0;
			while ((j < wrd) && (buf1[i + j] == buf2[j])) 
				++j;
			if (j == wrd) { 
				found = TRUE; 
				break; 
			}
		}
	}

	return found;
}


bool CHMSearchPanel::TitleSearch(const wxString& title, wxString& text,
				 bool caseSensitive, bool wholeWords)
{
	int i, j, wrd = text.Length();
	bool found = false;

	wxString tmp = title;
	int lng = tmp.Length();

	if (!caseSensitive) {
		tmp.MakeLower();
		text.MakeLower();
	}

	const wxChar *buf1 = tmp.c_str(), *buf2 = text.c_str();

	if(wholeWords) {
		for(i = 0; i < lng - wrd + 1; ++i) {

			if(WHITESPACE(buf1[i])) 
				continue;
			 			
			j = 0;
			while(buf1[i + j] == buf2[j] && j < wrd) 
				++j;

			if (j == wrd && (WHITESPACE(buf1[i + j]) || 
					 i+j == lng))
				if(i == 0 || WHITESPACE(buf1[i - 1])) {
					found = TRUE; 
					break; 
				}
			
		}
	} else {
		for (i = 0; i < lng - wrd + 1; ++i) {

			j = 0;
			while ((j < wrd) && (buf1[i + j] == buf2[(size_t)j])) 
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
	_results->Clear();
}


void CHMSearchPanel::SetConfig()
{
	wxConfig config(wxT("xchm"));	

	config.Write(wxT("/Search/caseSensitive"), (long)_case->GetValue());
	config.Write(wxT("/Search/wholeWordsOnly"), (long)_whole->GetValue());
	config.Write(wxT("/Search/titlesOnly"), (long)_titles->GetValue());
}

void CHMSearchPanel::GetConfig()
{
	long casesens, words, titles;
	wxConfig config(wxT("xchm"));

	if(config.Read(wxT("/Search/caseSensitive"), &casesens)) {
		
		config.Read(wxT("/Search/wholeWordsOnly"), &words);
		config.Read(wxT("/Search/titlesOnly"), &titles);

		_case->SetValue(casesens);
		_whole->SetValue(words);
		_titles->SetValue(titles);
	}

}


BEGIN_EVENT_TABLE(CHMSearchPanel, wxPanel)
    EVT_LISTBOX(ID_Results, CHMSearchPanel::OnSearchSel)
    EVT_BUTTON(ID_SearchButton, CHMSearchPanel::OnSearch)
    EVT_TEXT_ENTER(ID_SearchText, CHMSearchPanel::OnSearch)
END_EVENT_TABLE()


