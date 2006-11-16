/*

  This file has been received as a patch from Fritz Elfert
  <felfert@users.sourceforge.net>.
 
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


#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation
#endif

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/html/forcelnk.h>
#include <wx/html/m_templ.h>
#include <wx/fontenum.h>
#include <wx/tokenzr.h>
#include <math.h>

static void
unquote(wxString &s)
{
    if (s.StartsWith(wxT("'")) && s.Right(1).IsSameAs(wxT("'")))
        s = s.AfterFirst(wxT('\'')).BeforeLast(wxT('\''));
}

FORCE_LINK_ME(spantag)

TAG_HANDLER_BEGIN(SPAN, "SPAN")
    TAG_HANDLER_VARS
        int m_sztab[7];
        wxArrayString m_Faces;

    TAG_HANDLER_CONSTR(SPAN)
    {
        memset(m_sztab, 0, sizeof(m_sztab));
    }

    TAG_HANDLER_PROC(tag)
    {
        wxColour oldclr = m_WParser->GetActualColor();
        int oldsize = m_WParser->GetFontSize();
        wxString oldface = m_WParser->GetFontFace();

	if (tag.HasParam(wxT("STYLE"))) {
            wxStringTokenizer tk(tag.GetParam(wxT("STYLE")), wxT(";"));
            while (tk.HasMoreTokens()) {
                wxString attr = tk.GetNextToken().Strip(wxString::both);
                wxString aname = 
			attr.BeforeFirst(wxT(':')).Strip(wxString::both);

                if (aname.IsSameAs(wxT("font-size"), false)) {
                    wxString avalue = 
			    attr.AfterFirst(wxT(':')).Strip(wxString::both);
                    bool punit = false;
                    int n = oldsize;
                    long tmp;
                    if (avalue.IsSameAs(wxT("larger"), false))
                            avalue = wxT("+1");
                    if (avalue.IsSameAs(wxT("smaller"), false))
                            avalue = wxT("-1");
                    if (avalue.Right(2).IsSameAs(wxT("pt"))) {
                        punit = true;
                        avalue.Truncate(avalue.Length() - 2);
                    }
                    bool rel = (avalue.StartsWith(wxT("+")) 
				|| avalue.StartsWith(wxT("-")));
                    if (avalue.ToLong(&tmp, 10)) {
                        if (punit) {
                            if (m_sztab[0] == 0) {
                                // Calculate font-size-table from current font;
				    int sz = static_cast<int>(
					    m_WParser->CreateCurrentFont()
					    ->GetPointSize() / 
					    m_WParser->GetPixelScale());
                                switch (oldsize) {
                                    case 1:
                                        sz += 6;
                                        break;
                                    case 2:
                                        sz += 4;
                                        break;
                                    default:
                                    case 3:
                                        sz += 2;
                                        break;
                                    case 4:
                                        sz += 0;
                                        break;
                                    case 5:
                                        sz -= 2;
                                        break;
                                    case 6:
                                        sz -= 4;
                                        break;
                                    case 7:
                                        sz -= 6;
                                        break;
                                }
                                for (int i = -3; i <= 3; ++i)
                                    m_sztab[i+3] = sz + i * 2;
                            }
                            if (rel) {
                                if (oldsize < 1) {
                                    n = m_sztab[0];
                                } else if (oldsize > 6) {
                                    n = m_sztab[6];
                                } else n = m_sztab[oldsize];
                                n += tmp;
                            } else
                                n = tmp;

                            int dmin = 99999;
                            int midx = -1;
                            for (int i = 0; i < 7; i++) {
                                if (abs(m_sztab[i] - n) < dmin) {
                                    dmin = abs(m_sztab[i] - n);
                                    midx = i;
                                }
                            }
                            n = (midx != -1) ? (midx + 1) : oldsize;
                        } else {
                            if (rel)
                                n += tmp;
                            else
                                n = tmp;
                        }
                        m_WParser->SetFontSize(n);
                        m_WParser->GetContainer()->InsertCell(
                                new wxHtmlFontCell(
					m_WParser->CreateCurrentFont()));
                    }
                }

                if (aname.IsSameAs(wxT("font-family"), false)) {
                    if (m_Faces.GetCount() == 0) {
                        wxFontEnumerator enu;
                        enu.EnumerateFacenames();
#if wxMAJOR_VERSION == 2 && wxMINOR_VERSION > 7
			m_Faces = enu.GetFacenames();
#else
                        const wxArrayString *faces = enu.GetFacenames();
                        if (faces)
                            m_Faces = *faces;
#endif
                    }
                    wxStringTokenizer tk(
			    attr.AfterFirst(wxT(':')).Strip(wxString::both), 
			    wxT(","));

                    int index;
                    while (tk.HasMoreTokens()) {
                        wxString fn = tk.GetNextToken().Strip(wxString::both);
                        unquote(fn);
                        if (fn.IsSameAs(wxT("Andale Sans UI")))
                            fn = wxT("Andale Sans");
                        if((index = m_Faces.Index(fn, false)) != wxNOT_FOUND) {
                            m_WParser->SetFontFace(m_Faces[index]);
                            m_WParser->GetContainer()->InsertCell(
				    new wxHtmlFontCell(
					    m_WParser->CreateCurrentFont()));
                            break;
                        }
                    }
                }

                if (aname.IsSameAs(wxT("color"), false)) {
                    wxString avalue = 
			    attr.AfterFirst(wxT(':')).Strip(wxString::both);
                    unquote(avalue);
                    wxColour clr = oldclr;
                    if (avalue.GetChar(0) == wxT('#')) {
                        unsigned long tmp;
                        if (avalue.Mid(1).ToULong(&tmp, 16)) {
				clr = wxColour((unsigned char)(
						       (tmp & 0xFF0000) >> 16),
				       (unsigned char)((tmp & 0x00FF00) >> 8),
				       (unsigned char)(tmp & 0x0000FF));
                        }
                    } else {

// Handle colours defined in HTML 4.0:
#define HTML_COLOUR(name,r,g,b)		\
if (avalue.IsSameAs(wxT(name), false))	\
{ clr = wxColour(r,g,b); }
			    HTML_COLOUR("black",   0x00,0x00,0x00);
			    HTML_COLOUR("silver",  0xC0,0xC0,0xC0);
			    HTML_COLOUR("gray",    0x80,0x80,0x80);
			    HTML_COLOUR("white",   0xFF,0xFF,0xFF);
			    HTML_COLOUR("maroon",  0x80,0x00,0x00);
			    HTML_COLOUR("red",     0xFF,0x00,0x00);
			    HTML_COLOUR("purple",  0x80,0x00,0x80);
			    HTML_COLOUR("fuchsia", 0xFF,0x00,0xFF);
			    HTML_COLOUR("green",   0x00,0x80,0x00);
			    HTML_COLOUR("lime",    0x00,0xFF,0x00);
			    HTML_COLOUR("olive",   0x80,0x80,0x00);
			    HTML_COLOUR("yellow",  0xFF,0xFF,0x00);
			    HTML_COLOUR("navy",    0x00,0x00,0x80);
			    HTML_COLOUR("blue",    0x00,0x00,0xFF);
			    HTML_COLOUR("teal",    0x00,0x80,0x80);
			    HTML_COLOUR("aqua",    0x00,0xFF,0xFF);
#undef HTML_COLOUR
                    }
                    m_WParser->SetActualColor(clr);
                    m_WParser->GetContainer()->InsertCell(
			    new wxHtmlColourCell(clr));
                }

            }
        }

        ParseInner(tag);

        if (oldface != m_WParser->GetFontFace()) {
            m_WParser->SetFontFace(oldface);
            m_WParser->GetContainer()->InsertCell(
		    new wxHtmlFontCell(m_WParser->CreateCurrentFont()));
        }
        m_WParser->SetFontSize(oldsize);
        m_WParser->GetContainer()->InsertCell(
		new wxHtmlFontCell(m_WParser->CreateCurrentFont()));
        if (oldclr != m_WParser->GetActualColor()) {
            m_WParser->SetActualColor(oldclr);
            m_WParser->GetContainer()->InsertCell(
		    new wxHtmlColourCell(oldclr));
        }

        return true;
    }
TAG_HANDLER_END(SPAN)

TAGS_MODULE_BEGIN(XCHM)
    TAGS_MODULE_ADD(SPAN) 
TAGS_MODULE_END(XCHM)


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

