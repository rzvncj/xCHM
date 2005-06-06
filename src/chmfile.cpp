/*

  Copyright (C) 2003  Razvan Cojocaru <razvanco@gmx.net>
   XML-RPC/Context ID code contributed by Eamon Millman / PCI Geomatics
  <millman@pcigeomatics.com>
 
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


#include <chmfile.h>
#include <contenttaghandler.h>
#include <chmlistctrl.h>
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/strconv.h>
#include <wx/fontmap.h>
#include <wx/treectrl.h>

#include <assert.h>

#include <bitfiddle.inl>


namespace {

// damn wxWidgets and it's scoped ptr.
class UCharPtr {
public:
	UCharPtr(unsigned char *p) : _p(p) {}
	~UCharPtr() { delete[] _p; }

	unsigned char *get() { return _p; }

private:
	UCharPtr(const UCharPtr&);
	UCharPtr& operator=(const UCharPtr&);

private:
	unsigned char *_p;
};

} // namespace


// Big-enough buffer size for use with various routines.
#define BUF_SIZE 4096


// Thanks to Vadim Zeitlin.
#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204
#define MAC_CHARSET             77
#define BALTIC_CHARSET          186



CHMFile::CHMFile()
	: _chmFile(NULL), _home(wxT("/"))
{
}


CHMFile::CHMFile(const wxString& archiveName)
	: _chmFile(NULL), _home(wxT("/"))
{
	LoadCHM(archiveName);
}


CHMFile::~CHMFile()
{
	CloseCHM();
}


bool CHMFile::LoadCHM(const wxString&  archiveName)
{
	if(_chmFile)
		CloseCHM();

	assert(_chmFile == NULL);

	_chmFile = chm_open(static_cast<const char *>(archiveName.mb_str()));
	
	if(_chmFile == NULL)
		return false;

	_filename = archiveName;	
	GetArchiveInfo();
	LoadContextIDs();

	return true;
}



void CHMFile::CloseCHM()
{
	if(_chmFile == NULL)
		return;

	chm_close(_chmFile);
	
	_cidMap.clear();
	_chmFile = NULL;
	_home = wxT("/");
	_filename = _home = _topicsFile = _indexFile 
		= _title = _font = wxEmptyString;
}


bool CHMFile::GetTopicsTree(wxTreeCtrl *toBuild)
{
	chmUnitInfo ui;

	if(!toBuild)
		return false;

	if(_topicsFile.IsEmpty() || !ResolveObject(_topicsFile, &ui))
		return false;

	wxString src;
	src.Alloc(ui.length);
	GetFileAsString(src, &ui);

	if(src.IsEmpty())
		return false;

	ContentParser parser;
	parser.AddTagHandler(new ContentTagHandler(_enc,
#ifdef wxUSE_UNICODE
						   !_font.IsEmpty(),
#else
						   false,
#endif						
						   toBuild));	
	parser.Parse(src);

	return true;
}


bool CHMFile::GetIndex(CHMListCtrl* toBuild)
{
	chmUnitInfo ui;

	if(!toBuild)
		return false;

	if(_indexFile.IsEmpty() || !ResolveObject(_indexFile, &ui))
		return false;

	wxString src;
	src.Alloc(ui.length);
	GetFileAsString(src, &ui);

	if(src.IsEmpty())
		return false;

	ContentParser parser;
	parser.AddTagHandler(new ContentTagHandler(_enc,
#ifdef wxUSE_UNICODE
						   !_font.IsEmpty(),
#else
						   false,
#endif						
						   NULL, toBuild));

	parser.Parse(src);
	toBuild->UpdateUI();

	return true;
}


bool CHMFile::LoadContextIDs()
{
	chmUnitInfo ivb_ui, strs_ui;

	_cidMap.clear();

	// make sure what we need is there. 
	// #IVB has list of context ID's and #STRINGS offsets to file names.
	if( chm_resolve_object(_chmFile, "/#IVB", &ivb_ui ) != 
				CHM_RESOLVE_SUCCESS ||
		chm_resolve_object(_chmFile, "/#STRINGS", &strs_ui) != 
				CHM_RESOLVE_SUCCESS )
		return false; // failed to find internal files
	
	UCharPtr ivb_buf(new unsigned char[ivb_ui.length]);
	u_int64_t ivb_len = 0;

	if((ivb_len = chm_retrieve_object(_chmFile, &ivb_ui, 
					  ivb_buf.get(), 0, 
					  ivb_ui.length)) == 0 )
		return false; // failed to retrieve data

	// always odd (DWORD + 2(n)*DWORD, so make even
	ivb_len = ivb_len/sizeof(u_int32_t) - 1; 
	
	if( ivb_len % 2 != 0 )
		return false; // we retrieved unexpected data from the file.

	u_int32_t *ivbs = new u_int32_t[ivb_len];
	int j = 4; // offset to exclude first DWORD
	
	// convert our DWORDs to numbers
	for( unsigned int i = 0; i < ivb_len; i++ )
	{
		ivbs[i] = UINT32ARRAY(ivb_buf.get() + j);
		j+=4; // step to the next DWORD
	}

	UCharPtr strs_buf(new unsigned char[strs_ui.length]);
	u_int64_t strs_len = 0;

	if( (strs_len = chm_retrieve_object(_chmFile, &strs_ui, 
					    strs_buf.get(), 
					    0, strs_ui.length)) == 0 ) {
		delete[] ivbs;
		return false; // failed to retrieve data
	}

	for( unsigned int i = 0; i < ivb_len; i+=2 )
	{	// context-IDs as KEY, fileName from #STRINGS as VALUE
		_cidMap[ivbs[i]] = CURRENT_CHAR_STRING(
			strs_buf.get() + ivbs[i+1]);
	}

	delete[] ivbs;
	
	// everything went well!
	return true;
}

bool CHMFile::IsValidCID( const int contextID )
{
	if(_cidMap.empty())
		return FALSE;
	CHMIDMap::iterator itr = _cidMap.find( contextID );
	if( itr == _cidMap.end() )
		return FALSE;

	return TRUE;
}

wxString CHMFile::GetPageByCID( const int contextID )
{
	if(_cidMap.empty())
		return wxT("/");

	CHMIDMap::iterator itr = _cidMap.find( contextID );
	// make sure the key/value pair is valid
	if(itr == _cidMap.end() ) 
		return wxT("/");
	
	return wxString(wxT("/")) + itr->second;
}


bool CHMFile::IndexSearch(const wxString& text, bool wholeWords, 
			  bool titlesOnly, CHMSearchResults *results)
{
	bool partial = false;

	if(text.IsEmpty())
		return false;

	chmUnitInfo ui, uitopics, uiurltbl, uistrings, uiurlstr;
	if(::chm_resolve_object(_chmFile, "/$FIftiMain", &ui) !=
	   CHM_RESOLVE_SUCCESS || 
	   ::chm_resolve_object(_chmFile, "/#TOPICS", &uitopics) !=
	   CHM_RESOLVE_SUCCESS ||
	   ::chm_resolve_object(_chmFile, "/#STRINGS", &uistrings) !=
	   CHM_RESOLVE_SUCCESS ||
	   ::chm_resolve_object(_chmFile, "/#URLTBL", &uiurltbl) !=
	   CHM_RESOLVE_SUCCESS ||
	   ::chm_resolve_object(_chmFile, "/#URLSTR", &uiurlstr) !=
	   CHM_RESOLVE_SUCCESS)
		return false;

#define FTS_HEADER_LEN 0x32
	unsigned char header[FTS_HEADER_LEN];

	if(::chm_retrieve_object(_chmFile, &ui,
				 header, 0, FTS_HEADER_LEN) == 0)
		return false;
	
	unsigned char doc_index_s = header[0x1E], doc_index_r = header[0x1F];
	unsigned char code_count_s = header[0x20], code_count_r = header[0x21];
	unsigned char loc_codes_s = header[0x22], loc_codes_r = header[0x23];

	if(doc_index_s != 2 || code_count_s != 2 || loc_codes_s != 2) {
		// Don't know how to use values other than 2 yet. Maybe
		// next chmspec.
		return false;
	}

	unsigned char* cursor32 = header + 0x14;
	u_int32_t node_offset = UINT32ARRAY(cursor32);

	cursor32 = header + 0x2e;
	u_int32_t node_len = UINT32ARRAY(cursor32);

	unsigned char* cursor16 = header + 0x18;
	u_int16_t tree_depth = UINT16ARRAY(cursor16);

	unsigned char word_len, pos;
	wxString word;
	u_int32_t i = sizeof(u_int16_t);
	u_int16_t free_space;

	UCharPtr buffer(new unsigned char[node_len]);

	node_offset = GetLeafNodeOffset(text, node_offset, node_len,
					tree_depth, &ui);

	if(!node_offset) 
		return false;

	do {
		// got a leaf node here.
		if(::chm_retrieve_object(_chmFile, &ui, buffer.get(), 
					 node_offset, node_len) == 0)
			return false;

		cursor16 = buffer.get() + 6;
		free_space = UINT16ARRAY(cursor16);

		i = sizeof(u_int32_t) + sizeof(u_int16_t) + sizeof(u_int16_t);
		u_int64_t wlc_count, wlc_size;
		u_int32_t wlc_offset;

		while(i < node_len - free_space) {
			word_len = *(buffer.get() + i);
			pos = *(buffer.get() + i + 1);

			char *wrd_buf = new char[word_len];
			memcpy(wrd_buf, buffer.get() + i + 2, word_len - 1);
			wrd_buf[word_len - 1] = 0;

			if(pos == 0)
				word = CURRENT_CHAR_STRING(wrd_buf);
			else
				word = word.Mid(0, pos) +
					CURRENT_CHAR_STRING(wrd_buf);

			delete[] wrd_buf;

			i += 2 + word_len;
			unsigned char title = *(buffer.get() + i - 1);

			size_t encsz;
			wlc_count = be_encint(buffer.get() + i, encsz);
			i += encsz;
		
			cursor32 = buffer.get() + i;
			wlc_offset = UINT32ARRAY(cursor32);

			i += sizeof(u_int32_t) + sizeof(u_int16_t);
			wlc_size =  be_encint(buffer.get() + i, encsz);
			i += encsz;

			cursor32 = buffer.get();
			node_offset = UINT32ARRAY(cursor32);
		
			if(!title && titlesOnly)
				continue;

			if(wholeWords && !text.CmpNoCase(word))
				return ProcessWLC(wlc_count, wlc_size, 
						  wlc_offset, doc_index_s, 
						  doc_index_r,code_count_s, 
						  code_count_r, loc_codes_s, 
						  loc_codes_r, &ui, &uiurltbl,
						  &uistrings, &uitopics,
						  &uiurlstr, results);

			if(!wholeWords) {
				if(word.StartsWith(text.c_str())) {
					partial = true;
					ProcessWLC(wlc_count, wlc_size, 
						   wlc_offset, doc_index_s, 
						   doc_index_r,code_count_s, 
						   code_count_r, loc_codes_s, 
						   loc_codes_r, &ui, &uiurltbl,
						   &uistrings, &uitopics,
						   &uiurlstr, results);

				} else if(text.CmpNoCase(
						  // Mid() might be buggy.
						  word.Mid(0, text.Length()))
					  < -1)
					break;
			}

			if(results->size() >= MAX_SEARCH_RESULTS)
				break;
		}	
	} while(!wholeWords && word.StartsWith(text.c_str()) && node_offset);

	return partial;
}


bool CHMFile::ResolveObject(const wxString& fileName, chmUnitInfo *ui)
{
	return _chmFile != NULL && 
		::chm_resolve_object(_chmFile, 
				     static_cast<const char *>(
					     fileName.mb_str()), 
				     ui)
		== CHM_RESOLVE_SUCCESS;
}


size_t CHMFile::RetrieveObject(chmUnitInfo *ui, unsigned char *buffer,
			       off_t fileOffset, size_t bufferSize)
{
	return ::chm_retrieve_object(_chmFile, ui, buffer, fileOffset,
				     bufferSize);
}


bool CHMFile::GetArchiveInfo()
{
	bool retw = InfoFromWindows();
	bool rets = InfoFromSystem();

	return (retw || rets);
}


inline 
u_int32_t CHMFile::GetLeafNodeOffset(const wxString& text,
				     u_int32_t initialOffset,
				     u_int32_t buffSize,
				     u_int16_t treeDepth,
				     chmUnitInfo *ui)
{
	u_int32_t test_offset = 0;
	unsigned char* cursor16;
	unsigned char* cursor32;
	unsigned char word_len, pos;
	u_int32_t i = sizeof(u_int16_t);
	UCharPtr buffer(new unsigned char[buffSize]);
	wxString word;
	
	if(!buffer.get())
		return 0;

	while(--treeDepth) {
		if(initialOffset == test_offset)
			return 0;

		test_offset = initialOffset;
		if(::chm_retrieve_object(_chmFile, ui, buffer.get(), 
					 initialOffset, buffSize) == 0)
			return 0;

		cursor16 = buffer.get();
		u_int16_t free_space = UINT16ARRAY(cursor16);

		while(i < buffSize - free_space) {

			word_len = *(buffer.get() + i);
			pos = *(buffer.get() + i + 1);

			char *wrd_buf = new char[word_len];
			memcpy(wrd_buf, buffer.get() + i + 2, word_len - 1);
			wrd_buf[word_len - 1] = 0;

			if(pos == 0)
				word = CURRENT_CHAR_STRING(wrd_buf);
			else
				word = word.Mid(0, pos) +
					CURRENT_CHAR_STRING(wrd_buf);

			delete[] wrd_buf;

			if(text.CmpNoCase(word) <= 0) {
				cursor32 = buffer.get() + i + word_len + 1;
				initialOffset = UINT32ARRAY(cursor32);
				break;
			}

			i += word_len + sizeof(unsigned char) +
				+ sizeof(u_int32_t) + sizeof(u_int16_t);
		}
	}

	if(initialOffset == test_offset)
		return 0;

	return initialOffset;
}


inline 
bool CHMFile::ProcessWLC(u_int64_t wlc_count, u_int64_t wlc_size,
			 u_int32_t wlc_offset, unsigned char ds,
			 unsigned char dr, unsigned char cs,
			 unsigned char cr, unsigned char ls,
			 unsigned char lr, chmUnitInfo *uimain,
			 chmUnitInfo* uitbl, chmUnitInfo *uistrings,
			 chmUnitInfo* topics, chmUnitInfo *urlstr,
			 CHMSearchResults *results)
{
	int wlc_bit = 7;
	u_int64_t index = 0, count;
	size_t length, off = 0;
	UCharPtr buffer(new unsigned char[wlc_size]);
	unsigned char *cursor32;
	u_int32_t stroff, urloff;

#define TOPICS_ENTRY_LEN 16
	unsigned char entry[TOPICS_ENTRY_LEN];

#define COMMON_BUF_LEN 1025
	unsigned char combuf[COMMON_BUF_LEN];

	if(::chm_retrieve_object(_chmFile, uimain, buffer.get(), 
				 wlc_offset, wlc_size) == 0)
		return false;

	for(u_int64_t i = 0; i < wlc_count; ++i) {
		
		if(wlc_bit != 7) {
			++off;
			wlc_bit = 7;
		}

		index += sr_int(buffer.get() + off, &wlc_bit, ds, dr, length);
		off += length;

		if(::chm_retrieve_object(_chmFile, topics, entry, 
					 index * 16, TOPICS_ENTRY_LEN) == 0)
			return false;

		cursor32 = entry + 4;
		combuf[COMMON_BUF_LEN - 1] = 0;
		stroff = UINT32ARRAY(cursor32);

		wxString topic;

		if(::chm_retrieve_object(_chmFile, uistrings, combuf, 
					 stroff, COMMON_BUF_LEN - 1) == 0) {
			topic = wxString(_("Untitled in index"));

		} else {
			combuf[COMMON_BUF_LEN - 1] = 0;

#ifdef wxUSE_UNICODE
			if(_font.IsEmpty())
#endif
				topic = CURRENT_CHAR_STRING(combuf);
#ifdef wxUSE_UNICODE
			else {
				wxCSConv cv(wxFontMapper::Get()->
					    GetEncodingName(_enc));
				topic = wxString((const char *)combuf, cv);
			}
#endif
		}
	      
		cursor32 = entry + 8;
		urloff = UINT32ARRAY(cursor32);

		if(::chm_retrieve_object(_chmFile, uitbl, combuf, 
					 urloff, 12) == 0)
			return false;

		cursor32 = combuf + 8;
		urloff = UINT32ARRAY(cursor32);

		if(::chm_retrieve_object(_chmFile, urlstr, combuf, 
					 urloff + 8, COMMON_BUF_LEN - 1) == 0)
			return false;
	       
		combuf[COMMON_BUF_LEN - 1] = 0;

		wxString url = CURRENT_CHAR_STRING(combuf);

		if(!url.IsEmpty() && !topic.IsEmpty()) {
			if(results->size() >= MAX_SEARCH_RESULTS)
				return true;
			(*results)[url] = topic;
		}

		count = sr_int(buffer.get() + off, &wlc_bit, cs, cr, length);
		off += length;

		for(u_int64_t j = 0; j < count; ++j) {
			sr_int(buffer.get() + off, &wlc_bit, ls, lr, length);
			off += length;
		}
	}

	return true;
}


inline
void CHMFile::GetFileAsString(wxString& str, chmUnitInfo *ui)
{
#define BUF_SIZE2 1025
	char buffer[BUF_SIZE2];
	size_t ret = BUF_SIZE2 - 1, curr = 0;

	buffer[0] = '\0';

	do {
		ret = RetrieveObject(ui, reinterpret_cast<unsigned char *>(
					     buffer), curr, BUF_SIZE2 - 1);
		buffer[ret] = '\0';

		str.Append(CURRENT_CHAR_STRING(buffer));
		curr += ret;

	} while(ret == BUF_SIZE2 - 1);
}


inline
bool CHMFile::InfoFromWindows()
{
#define WIN_HEADER_LEN 0x08
	unsigned char buffer[BUF_SIZE];
	unsigned int factor;
	chmUnitInfo ui;
	long size = 0;

	if(::chm_resolve_object(_chmFile, "/#WINDOWS", &ui) == 
	   CHM_RESOLVE_SUCCESS) {
		if(!::chm_retrieve_object(_chmFile, &ui, 
		  			  buffer, 0, WIN_HEADER_LEN))
			return false;

		u_int32_t entries = *(u_int32_t *)(buffer);
		FIXENDIAN32(entries);
		u_int32_t entry_size = *(u_int32_t *)(buffer + 0x04);
		FIXENDIAN32(entry_size);
		
		UCharPtr uptr(new unsigned char[entries * entry_size]);
		unsigned char* raw = uptr.get();
		
		if(!::chm_retrieve_object(_chmFile, &ui, raw, 8, 
					  entries * entry_size))
			return false;

		if(::chm_resolve_object(_chmFile, "/#STRINGS", &ui) != 
		   CHM_RESOLVE_SUCCESS)
			return false;

		for(u_int32_t i = 0; i < entries; ++i) {

			u_int32_t offset = i * entry_size;

			u_int32_t off_title = 
				*(u_int32_t *)(raw + offset + 0x14);
			FIXENDIAN32(off_title);

			u_int32_t off_home = 
				*(u_int32_t *)(raw + offset + 0x68);
			FIXENDIAN32(off_home);

			u_int32_t off_hhc = 
				*(u_int32_t *)(raw + offset + 0x60);
			FIXENDIAN32(off_hhc);
			
			u_int32_t off_hhk = 
				*(u_int32_t *)(raw + offset + 0x64);
			FIXENDIAN32(off_hhk);

			factor = off_title / 4096;

			if(size == 0) 
				size = ::chm_retrieve_object(_chmFile, &ui, 
							     buffer, 
							     factor * 4096, 
							     BUF_SIZE);

			if(size && off_title)
				_title = CURRENT_CHAR_STRING(buffer + 
							     off_title % 4096);
			
			if(factor != off_home / 4096) {
				factor = off_home / 4096;		
				size = ::chm_retrieve_object(_chmFile, &ui, 
							     buffer, 
							     factor * 4096, 
							     BUF_SIZE);
			}
			
			if(size && off_home)
				_home = wxString(wxT("/")) +
					CURRENT_CHAR_STRING(buffer + 
							    off_home % 4096);
	       
			if(factor != off_hhc / 4096) {
				factor = off_hhc / 4096;
				size = ::chm_retrieve_object(_chmFile, &ui, 
							     buffer, 
							     factor * 4096, 
							     BUF_SIZE);
			}
		
			if(size && off_hhc) {
				_topicsFile = wxString(wxT("/")) + 
					CURRENT_CHAR_STRING(buffer + 
							    off_hhc % 4096);
			}

			if(factor != off_hhk / 4096) {
				factor = off_hhk / 4096;		
				size = ::chm_retrieve_object(_chmFile, &ui, 
							     buffer, 
							     factor * 4096, 
							     BUF_SIZE);
			}

			if(size && off_hhk)
				_indexFile = wxString(wxT("/")) + 
					CURRENT_CHAR_STRING(buffer + off_hhk 
							    % 4096);
		}
	}

	return true;
}


inline
bool CHMFile::InfoFromSystem()
{
	unsigned char buffer[BUF_SIZE];
	chmUnitInfo ui;
	
	int index = 0;
	unsigned char* cursor = NULL;
	u_int16_t value = 0;

	long size = 0;
	long cs = -1;

	// Do we have the #SYSTEM file in the archive?
	if(::chm_resolve_object(_chmFile, "/#SYSTEM", &ui) !=
	   CHM_RESOLVE_SUCCESS)
		return false;

	// Can we pull BUFF_SIZE bytes of the #SYSTEM file?
	if((size = ::chm_retrieve_object(_chmFile, &ui, buffer, 4, BUF_SIZE))
	   == 0)
		return false;

	buffer[size - 1] = 0;

	for(;;) {
		// This condition won't hold if I process anything
		// except NUL-terminated strings!
		if(index > size - 1 - (long)sizeof(u_int16_t))
			break;

		cursor = buffer + index;
		value = UINT16ARRAY(cursor);

		switch(value) {
		case 0:
			index += 2;
			cursor = buffer + index;
			
			if(_topicsFile.IsEmpty())
				_topicsFile = wxString(wxT("/")) 
					+ CURRENT_CHAR_STRING(buffer + 
							      index + 2);
			break;
		case 1:
			index += 2;
			cursor = buffer + index;

			if(_indexFile.IsEmpty())
				_indexFile = wxString(wxT("/"))
					+ CURRENT_CHAR_STRING(buffer + 
							      index + 2);
			break;
		case 2:
			index += 2;
			cursor = buffer + index;

			if(_home.IsEmpty() || _home == wxString(wxT("/")))
				_home = wxString(wxT("/"))
					+ CURRENT_CHAR_STRING(buffer + 
							      index + 2);
			break;
		case 3:
			index += 2;
			cursor = buffer + index;

			if(_title.IsEmpty())
				_title = CURRENT_CHAR_STRING(buffer + 
							     index + 2);
			break;
		case 6:
			index += 2;
			cursor = buffer + index;

			if(_topicsFile.IsEmpty()) {
				wxString topicAttempt = wxT("/"), tmp;
				topicAttempt += 
					CURRENT_CHAR_STRING(buffer +index +2);

				tmp = topicAttempt + wxT(".hhc");
				
				if(chm_resolve_object(_chmFile,
						      tmp.mb_str(), &ui)
				   == CHM_RESOLVE_SUCCESS)
					_topicsFile = tmp;

				tmp = topicAttempt + wxT(".hhk");
				
				if(chm_resolve_object(_chmFile,
						      tmp.mb_str(), &ui)
				   == CHM_RESOLVE_SUCCESS)
					_indexFile = tmp;
			}

			break;

		case 16:
			index += 2;
			cursor = buffer + index;

			_font = CURRENT_CHAR_STRING(buffer + index + 2);
			_font.AfterLast(wxT(',')).ToLong(&cs);
			_enc = GetFontEncFromCharSet(cs);
			
			break;
		default:
			index += 2;
			cursor = buffer + index;
		}

		value = UINT16ARRAY(cursor);
		index += value + 2;
	}

	return true;
}


inline
wxFontEncoding CHMFile::GetFontEncFromCharSet(int cs)
{
    wxFontEncoding fontEncoding;
            
    switch(cs) {
        case ANSI_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_1;
            break;            
        case EASTEUROPE_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_2;
            break;        
        case BALTIC_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_13;
            break;    
        case RUSSIAN_CHARSET:
            fontEncoding = wxFONTENCODING_KOI8;
            break;
        case ARABIC_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_6;
            break;
        case GREEK_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_7;
            break;
        case HEBREW_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_8;
            break;
        case TURKISH_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_9;
            break;
        case THAI_CHARSET:
            fontEncoding = wxFONTENCODING_ISO8859_11;
            break;
        case SHIFTJIS_CHARSET:
            fontEncoding = wxFONTENCODING_CP932;
            break;
        case GB2312_CHARSET:
            fontEncoding = wxFONTENCODING_CP936;
            break;
        case HANGUL_CHARSET:
            fontEncoding = wxFONTENCODING_CP949;
            break;
        case CHINESEBIG5_CHARSET:
            fontEncoding = wxFONTENCODING_CP950;
            break;
        case OEM_CHARSET:
            fontEncoding = wxFONTENCODING_CP437;
            break;
        default:
            // assume the system charset
            fontEncoding = wxFONTENCODING_SYSTEM;
            break;            
    }
    
    return fontEncoding;
}





