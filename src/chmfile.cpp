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


#include <chmfile.h>
#include <contenttaghandler.h>
#include <wx/defs.h>
#include <wx/strconv.h>
#include <wx/tokenzr.h>
#include <wx/ptr_scpd.h>
#include <assert.h>


#include <bitfiddle.inl>


namespace {

// I'm defining a smart pointer class called wxCharArray, wxWindows style.
wxDECLARE_SCOPED_ARRAY(unsigned char, UCharPtr)
wxDEFINE_SCOPED_ARRAY(unsigned char, UCharPtr)

} // namespace


#ifdef wxUSE_UNICODE

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x), wxConvISO8859_1)

#else

#	define CURRENT_CHAR_STRING(x) \
	wxString(reinterpret_cast<const char *>(x))

#endif



CHMFile::CHMFile()
	: _chmFile(NULL), _home(wxT("/"))
{}


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

	return true;
}



void CHMFile::CloseCHM()
{
	if(_chmFile == NULL)
		return;

	chm_close(_chmFile);
	
	_chmFile = NULL;
	_home = wxT("/");
	_filename = _home = _topicsFile = _indexFile 
		= _title = wxEmptyString;
}


bool CHMFile::GetTopicsTree(wxTreeCtrl *toBuild)
{
#define BUF_SIZE2 1025
	chmUnitInfo ui;
	char buffer[BUF_SIZE2];
	size_t ret = BUF_SIZE2 - 1, curr = 0;

	if(!toBuild)
		return false;

	if(_topicsFile.IsEmpty() && _indexFile.IsEmpty())
		return false;


	if(_topicsFile.IsEmpty() || !ResolveObject(_topicsFile, &ui))
		   if(_indexFile.IsEmpty() || !ResolveObject(_indexFile, &ui))
			return false;

	buffer[0] = '\0';

	wxString src;
	src.Alloc(ui.length);

	do {
		ret = RetrieveObject(&ui, reinterpret_cast<unsigned char *>(
					     buffer), curr, BUF_SIZE2 - 1);
		buffer[ret] = '\0';

		src.Append(CURRENT_CHAR_STRING(buffer));
		curr += ret;

	} while(ret == BUF_SIZE2 - 1);
	
	if(src.IsEmpty())
		return false;

	ContentParser parser;
	parser.AddTagHandler(new ContentTagHandler(toBuild));

	parser.Parse(src);
	toBuild->Expand(toBuild->GetRootItem());

	return true;

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

	u_int32_t* cursor32 = reinterpret_cast<u_int32_t*>(header + 0x14);
	u_int32_t node_offset = *cursor32;
	FIXENDIAN32(node_offset);

	cursor32 = reinterpret_cast<u_int32_t*>(header + 0x2e);
	u_int32_t node_len = *cursor32;
	FIXENDIAN32(node_len);

	u_int16_t* cursor16 = reinterpret_cast<u_int16_t*>(header + 0x18);
	u_int16_t tree_depth = *cursor16;
	FIXENDIAN16(tree_depth);

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

		cursor16 = reinterpret_cast<u_int16_t *>(buffer.get() + 6);
		free_space = *cursor16;
		FIXENDIAN16(free_space);

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
		
			cursor32 = reinterpret_cast<u_int32_t *>(
				buffer.get() + i);
			wlc_offset = *cursor32;
			FIXENDIAN32(wlc_offset);

			i += sizeof(u_int32_t) + sizeof(u_int16_t);
			wlc_size =  be_encint(buffer.get() + i, encsz);
			i += encsz;

			cursor32 = reinterpret_cast<u_int32_t*>(buffer.get());
			node_offset = *cursor32;
			FIXENDIAN32(node_offset);
		
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
#define BUF_SIZE 2048
	unsigned char buffer[BUF_SIZE];
	chmUnitInfo ui;
	
	int index = 0;
	u_int16_t *cursor = NULL;
	long size = 0;

	// Do we have the #SYSTEM file in the archive?
	if(::chm_resolve_object(_chmFile, "/#SYSTEM", &ui) !=
	   CHM_RESOLVE_SUCCESS)
		return false;

	// Can we pull BUFF_SIZE bytes of the #SYSTEM file?
	if((size = ::chm_retrieve_object(_chmFile, &ui, buffer, 4, BUF_SIZE))
	   == 0)
		return false;

	for(;;) {

		if(index > size - 1 - (long)sizeof(u_int16_t))
			break;

		cursor = (u_int16_t *)(buffer + index);
		FIXENDIAN16(*cursor);

		switch(*cursor) {
		case 0:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_topicsFile = wxString(wxT("/")) 
				+ CURRENT_CHAR_STRING(buffer + index + 2);
			index += *cursor + 2;
			break;
		case 1:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_indexFile = wxString(wxT("/"))
				+ CURRENT_CHAR_STRING(buffer + index + 2);
			index += *cursor + 2;
			break;
		case 2:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_home = wxString(wxT("/"))
				+ CURRENT_CHAR_STRING(buffer + index + 2);
			index += *cursor + 2;
			break;
		case 3:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_title = CURRENT_CHAR_STRING(buffer + index + 2);
			index += *cursor + 2;
			break;
		case 6:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);

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

			index += *cursor + 2;
			break;
		default:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);
			index += *cursor + 2;
		}
	}

	// one last try to see if we can get contents from "/#STRINGS"
	if(_topicsFile.IsEmpty() && _indexFile.IsEmpty()) {

		index = 0;
		wxString chunk;

		if(::chm_resolve_object(_chmFile, "/#STRINGS", &ui) !=
		   CHM_RESOLVE_SUCCESS)
			// no /#STRINGS, but we did get some info.
			return true;

		// Can we pull BUFF_SIZE bytes of the #SYSTEM file?
		if(::chm_retrieve_object(_chmFile, &ui,
				       buffer, 1, BUF_SIZE) == 0)
			// this should never happen, the specs say
			// the least the file can be is 4096 bytes.
			return true;

		// There's that 9 again :).
		for(int i=0; i<9; ++i) {
			chunk = CURRENT_CHAR_STRING(buffer + index);
			
			if(chunk.Right(4).Lower() == wxT(".hhc")) {
				_topicsFile = wxString(wxT("/")) + chunk;
				return true;

			} else if(chunk.Right(4).Lower() == wxT(".hhk")) {
				_indexFile = wxString(wxT("/")) + chunk;
				return true;
			}

			index += chunk.size() + 1;
		}
		
	}

	return true;
}


inline 
u_int32_t CHMFile::GetLeafNodeOffset(const wxString& text,
				     u_int32_t initialOffset,
				     u_int32_t buffSize,
				     u_int16_t treeDepth,
				     chmUnitInfo *ui)
{
	u_int32_t test_offset = 0;
	u_int16_t* cursor16;
	u_int32_t* cursor32;
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

		cursor16 = reinterpret_cast<u_int16_t*>(buffer.get());
		u_int16_t free_space = *cursor16;
		FIXENDIAN16(free_space);

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
				cursor32 = reinterpret_cast<u_int32_t*>(
					buffer.get() + i + word_len + 1);
				initialOffset = *cursor32;
				FIXENDIAN32(initialOffset);
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
	u_int32_t *cursor32;
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

		cursor32 = reinterpret_cast<u_int32_t *>(entry + 4);
		combuf[COMMON_BUF_LEN - 1] = 0;
		stroff = *cursor32;
		FIXENDIAN32(stroff);

		if(::chm_retrieve_object(_chmFile, uistrings, combuf, 
					 stroff, COMMON_BUF_LEN - 1) == 0)
			return false;

		combuf[COMMON_BUF_LEN - 1] = 0;
		wxString topic = CURRENT_CHAR_STRING(combuf);
	      
		cursor32 = reinterpret_cast<u_int32_t *>(entry + 8);
		urloff = *cursor32;
		FIXENDIAN32(urloff);

		if(::chm_retrieve_object(_chmFile, uitbl, combuf, 
					 urloff, 12) == 0)
			return false;

		cursor32 = reinterpret_cast<u_int32_t*>(combuf + 8);
		urloff = *cursor32;
		FIXENDIAN32(urloff);

		if(::chm_retrieve_object(_chmFile, urlstr, combuf, 
					 urloff + 8, COMMON_BUF_LEN - 1) == 0)
			return false;
		combuf[COMMON_BUF_LEN - 1] = 0;

		wxString url = CURRENT_CHAR_STRING(combuf);

		if(!url.IsEmpty() && !topic.IsEmpty())
			(*results)[url] = topic;

		count = sr_int(buffer.get() + off, &wlc_bit, cs, cr, length);
		off += length;

		for(u_int64_t j = 0; j < count; ++j) {
			sr_int(buffer.get() + off, &wlc_bit, ls, lr, length);
			off += length;
		}
	}

	return true;
}






