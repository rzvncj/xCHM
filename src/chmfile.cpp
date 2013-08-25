/*

  Copyright (C) 2003 - 2013  Razvan Cojocaru <rzvncj@gmail.com>
  XML-RPC/Context ID code contributed by Eamon Millman / PCI Geomatics
    <millman@pcigeomatics.com>
  Bugfixes contributed by Kuang-che Wu 
    https://sourceforge.net/users/kcwu/
 
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


#include <chmfile.h>
#include <chmlistctrl.h>
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/strconv.h>
#include <wx/fontmap.h>
#include <wx/treectrl.h>
#include <wx/progdlg.h>

#include <assert.h>

#include <bitfiddle.inl>
#include <hhcparser.h>


// damn wxWidgets and it's scoped ptr.
class UCharPtr {
public:
	UCharPtr(unsigned char *p, size_t size = 0) : _p(p), _size(size) {}
	~UCharPtr() { delete[] _p; }

	unsigned char *get() { return _p; }
	size_t size() const { return _size; }

private:
	UCharPtr(const UCharPtr&);
	UCharPtr& operator=(const UCharPtr&);

private:
	unsigned char *_p;
	size_t _size;
};


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


// Hello, Microsoft
#define LANG_NEUTRAL		0x00 // check
#define LANG_ARABIC		0x01 // check
#define LANG_BULGARIAN		0x02 // check
#define LANG_CATALAN		0x03
#define LANG_CHINESE		0x04 // check
#define LANG_CZECH		0x05
#define LANG_DANISH		0x06
#define LANG_GERMAN		0x07
#define LANG_GREEK		0x08 // check
#define LANG_ENGLISH		0x09
#define LANG_SPANISH		0x0a
#define LANG_FINNISH		0x0b
#define LANG_FRENCH		0x0c
#define LANG_HEBREW		0x0d // check
#define LANG_HUNGARIAN		0x0e
#define LANG_ICELANDIC		0x0f
#define LANG_ITALIAN		0x10
#define LANG_JAPANESE		0x11 // check
#define LANG_KOREAN		0x12 // check
#define LANG_DUTCH		0x13
#define LANG_NORWEGIAN		0x14
#define LANG_POLISH		0x15
#define LANG_PORTUGUESE		0x16
#define LANG_ROMANIAN		0x18
#define LANG_RUSSIAN		0x19 // check
#define LANG_CROATIAN		0x1a
#define LANG_SERBIAN		0x1a
#define LANG_SLOVAK		0x1b
#define LANG_ALBANIAN		0x1c
#define LANG_SWEDISH		0x1d
#define LANG_THAI		0x1e // check
#define LANG_TURKISH		0x1f // check
#define LANG_URDU		0x20
#define LANG_INDONESIAN		0x21
#define LANG_UKRAINIAN		0x22 // check
#define LANG_BELARUSIAN		0x23
#define LANG_SLOVENIAN		0x24
#define LANG_ESTONIAN		0x25
#define LANG_LATVIAN		0x26
#define LANG_LITHUANIAN		0x27
#define LANG_FARSI		0x29
#define LANG_VIETNAMESE		0x2a
#define LANG_ARMENIAN		0x2b
#define LANG_AZERI		0x2c
#define LANG_BASQUE		0x2d
#define LANG_MACEDONIAN		0x2f
#define LANG_AFRIKAANS		0x36
#define LANG_GEORGIAN		0x37
#define LANG_FAEROESE		0x38
#define LANG_HINDI		0x39
#define LANG_MALAY		0x3e
#define LANG_KAZAK		0x3f
#define LANG_KYRGYZ		0x40
#define LANG_SWAHILI		0x41
#define LANG_UZBEK		0x43
#define LANG_TATAR		0x44
#define LANG_BENGALI		0x45
#define LANG_PUNJABI		0x46
#define LANG_GUJARATI		0x47
#define LANG_ORIYA		0x48
#define LANG_TAMIL		0x49
#define LANG_TELUGU		0x4a
#define LANG_KANNADA		0x4b
#define LANG_MALAYALAM		0x4c
#define LANG_ASSAMESE		0x4d
#define LANG_MARATHI		0x4e
#define LANG_SANSKRIT		0x4f
#define LANG_MONGOLIAN		0x50
#define LANG_GALICIAN		0x56
#define LANG_KONKANI		0x57
#define LANG_MANIPURI		0x58
#define LANG_SINDHI		0x59
#define LANG_SYRIAC		0x5a
#define LANG_KASHMIRI		0x60
#define LANG_NEPALI		0x61
#define LANG_DIVEHI		0x65


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

	_enc = wxFONTENCODING_SYSTEM;
	_filename = archiveName;	
	GetArchiveInfo();
	LoadContextIDs();

#if wxUSE_UNICODE
	_title = translateEncoding(_title, _enc);
#endif
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
	_filename = _home = _topicsFile = _indexFile = _title 
		= _font = wxEmptyString;
}


#define MSG_RETR_TOC _("Retrieving table of contents..")
#define MSG_RETR_IDX _("Retrieving index..")
#define EMPTY_INDEX _("Untitled in index")


bool CHMFile::BinaryTOC(wxTreeCtrl *toBuild)
{
	if(!toBuild)
		return false;

	chmUnitInfo ti_ui, ts_ui, st_ui, ut_ui, us_ui;
	
	if(::chm_resolve_object(_chmFile, "/#TOCIDX", &ti_ui ) != 
	   CHM_RESOLVE_SUCCESS 
	   || ::chm_resolve_object(_chmFile, "/#TOPICS", &ts_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#STRINGS", &st_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#URLTBL", &ut_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#URLSTR", &us_ui) != 
	   CHM_RESOLVE_SUCCESS)
		return false; // failed to find internal files

	if(ti_ui.length < 4) // just make sure
		return false;

	UCharPtr  topidx(new unsigned char[ti_ui.length], ti_ui.length);
	UCharPtr  topics(new unsigned char[ts_ui.length], ts_ui.length);
	UCharPtr strings(new unsigned char[st_ui.length], st_ui.length);
	UCharPtr  urltbl(new unsigned char[ut_ui.length], ut_ui.length);
	UCharPtr  urlstr(new unsigned char[us_ui.length], us_ui.length);
	
	if(::chm_retrieve_object(_chmFile, &ti_ui, topidx.get(), 
				 0, ti_ui.length) != (int64_t)ti_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &ts_ui, topics.get(), 
				 0, ts_ui.length) != (int64_t)ts_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &st_ui, strings.get(), 
				 0, st_ui.length) != (int64_t)st_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &ut_ui, urltbl.get(), 0, 
				 ut_ui.length) != (int64_t)ut_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &us_ui, urlstr.get(), 0, 
				 us_ui.length) != (int64_t)us_ui.length)
		return false;

	uint32_t off = UINT32ARRAY(topidx.get());
	RecurseLoadBTOC(topidx, topics, strings, urltbl, urlstr, off, 
			toBuild, 1);

	return true;
}


void CHMFile::RecurseLoadBTOC(UCharPtr& topidx, UCharPtr& topics,
			      UCharPtr& strings, UCharPtr& urltbl,
			      UCharPtr& urlstr, uint32_t offset,
			      wxTreeCtrl *toBuild, int level)
{
	while(offset) {
		if(topidx.size() < offset + 20)
			return;

		uint32_t flags = UINT32ARRAY(topidx.get() + offset + 4);
		uint32_t index = UINT32ARRAY(topidx.get() + offset + 8);

		if((flags & 0x4) || (flags & 0x8)) { // book or local
			if(!GetItem(topics, strings, urltbl, urlstr, index,
				    toBuild, NULL, wxEmptyString, level, 
				    (flags & 0x8) == 0))
				return;
		}

		if(flags & 0x4) { // book

			if(topidx.size() < offset + 24)
				return;

			uint32_t child = UINT32ARRAY(topidx.get() + offset
						      + 20);
			if(child) {
				RecurseLoadBTOC(topidx, topics, strings, 
						urltbl, urlstr, child,
						toBuild, level + 1); 
			}
		}

		offset = UINT32ARRAY(topidx.get() + offset + 0x10);
	}
}


bool CHMFile::GetItem(UCharPtr& topics, UCharPtr& strings, UCharPtr& urltbl,
		      UCharPtr& urlstr, uint32_t index, 
		      wxTreeCtrl *tree, CHMListCtrl* list, 
		      const wxString& idxName, 
		      int level, bool local)
{
	static wxTreeItemId parents[TREE_BUF_SIZE];

	static int calls = 0;
	static const int yieldTime = 256;

	++calls;
	if(calls % yieldTime) {
		calls = 0;
		wxYield();
	}

	if(tree)
		parents[0] = tree->GetRootItem();

	std::string name, value;

	if(local) {
		if(strings.size() < index + 1)
			return false;
		name = (char *)(strings.get() + index);

	} else {

		if(topics.size() < (index * 16) + 12)
			return false;

		uint32_t offset = UINT32ARRAY(topics.get()+ (index * 16) + 4);
		int32_t test = (int32_t)offset;

		if(strings.size() < offset + 1)
			return false;

		if(test == -1)
			return false;

		if(!list)
			name = (char *)(strings.get() + offset);

		// #URLTBL index
		offset = UINT32ARRAY(topics.get() + (index * 16) + 8);
	
		if(urltbl.size() < offset + 12)
			return false;
		
		offset = UINT32ARRAY(urltbl.get() + offset + 8);
		
		if(urlstr.size() < offset)
			return false;

		value = (char *)(urlstr.get() + offset + 8);
	}

	if(!value.empty() && value[0] != '/')
		value = std::string("/") + value;
	
	wxString tname;
	wxString tvalue = CURRENT_CHAR_STRING(value.c_str());

	if(tree && !name.empty()) {
		int parentIndex = level ? level - 1 : 0;
		tname = translateEncoding(CURRENT_CHAR_STRING(name.c_str()),
			_enc);

		parents[level] = 
			tree->AppendItem(parents[parentIndex], tname, 2, 2,
					    new URLTreeItem(tvalue));
		if(level) {
			if(tree->GetItemImage(parents[parentIndex]) != 0) {
				tree->SetItemImage(parents[parentIndex], 0,
						   wxTreeItemIcon_Normal);
				
				tree->SetItemImage(parents[parentIndex], 0,
						   wxTreeItemIcon_Selected);
				
				tree->SetItemImage(parents[parentIndex], 1,
						   wxTreeItemIcon_Expanded);
			}
		}
	}

	if(list) {
		tname = idxName;
		if(!value.empty()) {
			if(tname.IsEmpty())
				tname = EMPTY_INDEX;
		}

		list->AddPairItem(tname, tvalue);
	}
	
	return true;
}



bool CHMFile::GetTopicsTree(wxTreeCtrl *toBuild)
{
	chmUnitInfo ui;
	char buffer[BUF_SIZE];
	size_t ret = BUF_SIZE - 1, curr = 0;
       
	if(!toBuild) 
		return false;

	toBuild->Freeze();
	bool btoc = BinaryTOC(toBuild);
	toBuild->Thaw();

	if(btoc)
		return true;

	// Fall back to parsing the HTML TOC file, if that's in the archive
	if(_topicsFile.IsEmpty() || !ResolveObject(_topicsFile, &ui)) 
		return false;

	toBuild->Freeze();

	buffer[0] = '\0';

	HHCParser p(_enc, toBuild, NULL);

	do {
		ret = RetrieveObject(&ui, reinterpret_cast<unsigned char *>(
					     buffer), curr, BUF_SIZE - 1);
		buffer[ret] = '\0';

		p.parse(buffer);
		curr += ret;

	} while(ret == BUF_SIZE - 1);

	toBuild->Thaw();

	return true;
}


bool CHMFile::ConvertFromUnicode(std::string& value, unsigned char* buffer,
				 size_t bufferLength)
{
	size_t offset = 0;
	uint16_t elem = 1;
	value = "";

	while(bufferLength >= offset + sizeof(elem)) {
		elem = UINT16ARRAY(buffer + offset);

		if(elem != 0) {
			value += *((char *)(buffer + offset));
		} else {
			return true;
		}

		offset += sizeof(elem);
	}

	return false;
}


// This function is too long: prime candidate for refactoring someday
bool CHMFile::BinaryIndex(CHMListCtrl* toBuild, const wxCSConv& cv)
{
	if(!toBuild)
		return false;

	chmUnitInfo bt_ui, ts_ui, st_ui, ut_ui, us_ui;
	unsigned long items = 0;
		
	if(::chm_resolve_object(_chmFile, "/$WWKeywordLinks/BTree",
				&bt_ui ) != CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#TOPICS", &ts_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#STRINGS", &st_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#URLTBL", &ut_ui) != 
	   CHM_RESOLVE_SUCCESS
	   || ::chm_resolve_object(_chmFile, "/#URLSTR", &us_ui) != 
	   CHM_RESOLVE_SUCCESS)
		return false; // failed to find internal files

	UCharPtr  btree(new unsigned char[bt_ui.length], bt_ui.length);
	UCharPtr  topics(new unsigned char[ts_ui.length], ts_ui.length);
	UCharPtr strings(new unsigned char[st_ui.length], st_ui.length);
	UCharPtr  urltbl(new unsigned char[ut_ui.length], ut_ui.length);
	UCharPtr  urlstr(new unsigned char[us_ui.length], us_ui.length);

	if(::chm_retrieve_object(_chmFile, &bt_ui, btree.get(), 
				 0, bt_ui.length) != (int64_t)bt_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &ts_ui, topics.get(), 
				 0, ts_ui.length) != (int64_t)ts_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &st_ui, strings.get(), 
				 0, st_ui.length) != (int64_t)st_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &ut_ui, urltbl.get(), 0, 
				 ut_ui.length) != (int64_t)ut_ui.length)
		return false;

	if(::chm_retrieve_object(_chmFile, &us_ui, urlstr.get(), 0, 
				 us_ui.length) != (int64_t)us_ui.length)
		return false;


	if(bt_ui.length < 0x4c + 12)
		return false;

	unsigned long offset = 0x4c;
	int32_t next = -1;
	int16_t freeSpace, spaceLeft;
	const short blockSize = 2048;

	do {
		if(bt_ui.length < offset + 12)
			return (items != 0); // end of buffer

		freeSpace = UINT16ARRAY(btree.get() + offset);
		next = INT32ARRAY(btree.get() + offset + 8);
		spaceLeft = blockSize - 12;
		offset += 12;

		while(spaceLeft > freeSpace) {

			uint16_t tmp = 0;
			wxString name;

			do { // accumulate the index name
				if(bt_ui.length < offset 
				   + sizeof(uint16_t))
					return items != 0;
				
				tmp = UINT16ARRAY(btree.get() 
						  + offset);
				offset += sizeof(uint16_t);
				spaceLeft -= sizeof(uint16_t);

				name.Append(charForCode(tmp, cv, true));
				
			} while(tmp != 0);

			if(bt_ui.length < offset + 16)
				return (items != 0);
			
			uint16_t seeAlso = UINT16ARRAY(btree.get() + offset);
			uint32_t numTopics = UINT32ARRAY(btree.get() + offset
							  + 0xc);
			offset += 16;
			spaceLeft -= 16;

			if(seeAlso) {

				// TODO: something about this duplicated code
				do { // get over the Unicode string
					if(bt_ui.length < offset 
					   + sizeof(uint16_t))
						return items != 0;

					tmp = UINT16ARRAY(btree.get() 
							  + offset);
					offset += sizeof(uint16_t);
					spaceLeft -= sizeof(uint16_t);
					
				} while(tmp != 0);
			} else {

				for(uint32_t i = 0; i < numTopics
					    && spaceLeft > freeSpace; ++i) {
					if(bt_ui.length < offset
					   + sizeof(uint32_t))
						return (items != 0);

					uint32_t index = 
						UINT32ARRAY(btree.get() 
							    + offset);

					GetItem(topics, strings, urltbl,
						urlstr, index, NULL, 
						toBuild, name, 0, false);
					++items;

					offset += sizeof(uint32_t);
					spaceLeft -= sizeof(uint32_t);
				}
			}

			if(bt_ui.length < offset + 8)
				return (items != 0);

			offset += 8;
			spaceLeft -= 8;
		}
		
		offset += spaceLeft;

	} while(next != -1);

	return (items != 0);
}


bool CHMFile::GetIndex(CHMListCtrl* toBuild)
{
	chmUnitInfo ui;
	char buffer[BUF_SIZE];
	size_t ret = BUF_SIZE - 1, curr = 0;

	if(!toBuild)
		return false;

	std::auto_ptr<wxCSConv> cvPtr;
	createCSConvPtr(cvPtr, _enc);

	toBuild->Freeze();
	bool bindex = BinaryIndex(toBuild, *cvPtr);
	toBuild->Thaw();

	if(bindex)
		return true;

	if(_indexFile.IsEmpty() || !ResolveObject(_indexFile, &ui))
		return false;

	toBuild->Freeze();

	buffer[0] = '\0';

	HHCParser p(_enc, NULL, toBuild);

	do {
		ret = RetrieveObject(&ui, reinterpret_cast<unsigned char *>(
					     buffer), curr, BUF_SIZE - 1);
		buffer[ret] = '\0';

		p.parse(buffer);
		curr += ret;

	} while(ret == BUF_SIZE - 1);
	
	toBuild->Thaw();
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
	uint64_t ivb_len = 0;

	if((ivb_len = chm_retrieve_object(_chmFile, &ivb_ui, 
					  ivb_buf.get(), 0, 
					  ivb_ui.length)) == 0 )
		return false; // failed to retrieve data

	// always odd (DWORD + 2(n)*DWORD, so make even
	ivb_len = ivb_len/sizeof(uint32_t) - 1; 
	
	if( ivb_len % 2 != 0 )
		return false; // we retrieved unexpected data from the file.

	uint32_t *ivbs = new uint32_t[ivb_len];
	int j = 4; // offset to exclude first DWORD
	
	// convert our DWORDs to numbers
	for( unsigned int i = 0; i < ivb_len; i++ )
	{
		ivbs[i] = UINT32ARRAY(ivb_buf.get() + j);
		j+=4; // step to the next DWORD
	}

	UCharPtr strs_buf(new unsigned char[strs_ui.length]);
	uint64_t strs_len = 0;

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
	uint32_t node_offset = UINT32ARRAY(cursor32);

	cursor32 = header + 0x2e;
	uint32_t node_len = UINT32ARRAY(cursor32);

	unsigned char* cursor16 = header + 0x18;
	uint16_t tree_depth = UINT16ARRAY(cursor16);

	unsigned char word_len, pos;
	wxString word;
	uint32_t i = sizeof(uint16_t);
	uint16_t free_space;

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

		i = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t);
		uint64_t wlc_count, wlc_size;
		uint32_t wlc_offset;

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

			i += sizeof(uint32_t) + sizeof(uint16_t);
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
	// Order counts, parse #SYSTEM before #WINDOWS
	// (thanks to Kuang-che Wu for pointing that out)
	bool rets = InfoFromSystem();
	bool retw = InfoFromWindows();

	return (retw || rets);
}


uint32_t CHMFile::GetLeafNodeOffset(const wxString& text,
				     uint32_t initialOffset,
				     uint32_t buffSize,
				     uint16_t treeDepth,
				     chmUnitInfo *ui)
{
	uint32_t test_offset = 0;
	unsigned char* cursor16;
	unsigned char* cursor32;
	unsigned char word_len, pos;
	uint32_t i = sizeof(uint16_t);
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
		uint16_t free_space = UINT16ARRAY(cursor16);

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
				+ sizeof(uint32_t) + sizeof(uint16_t);
		}
	}

	if(initialOffset == test_offset)
		return 0;

	return initialOffset;
}


bool CHMFile::ProcessWLC(uint64_t wlc_count, uint64_t wlc_size,
			 uint32_t wlc_offset, unsigned char ds,
			 unsigned char dr, unsigned char cs,
			 unsigned char cr, unsigned char ls,
			 unsigned char lr, chmUnitInfo *uimain,
			 chmUnitInfo* uitbl, chmUnitInfo *uistrings,
			 chmUnitInfo* topics, chmUnitInfo *urlstr,
			 CHMSearchResults *results)
{
	int wlc_bit = 7;
	uint64_t index = 0, count;
	size_t length, off = 0;
	UCharPtr buffer(new unsigned char[wlc_size]);
	unsigned char *cursor32;
	uint32_t stroff, urloff;

#define TOPICS_ENTRY_LEN 16
	unsigned char entry[TOPICS_ENTRY_LEN];

#define COMMON_BUF_LEN 1025
	unsigned char combuf[COMMON_BUF_LEN];

	if(::chm_retrieve_object(_chmFile, uimain, buffer.get(), 
				 wlc_offset, wlc_size) == 0)
		return false;

	for(uint64_t i = 0; i < wlc_count; ++i) {
		
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
			topic = EMPTY_INDEX;

		} else {
			combuf[COMMON_BUF_LEN - 1] = 0;

#if wxUSE_UNICODE
			if(_enc == wxFONTENCODING_SYSTEM)
#endif
				topic = CURRENT_CHAR_STRING(combuf);
#if wxUSE_UNICODE
			else {
				std::auto_ptr<wxCSConv> cvPtr;
				createCSConvPtr(cvPtr, _enc);
				topic = wxString((const char *)combuf, *cvPtr);
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

		for(uint64_t j = 0; j < count; ++j) {
			sr_int(buffer.get() + off, &wlc_bit, ls, lr, length);
			off += length;
		}
	}

	return true;
}


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

		uint32_t entries = UINT32ARRAY(buffer);
		uint32_t entry_size = UINT32ARRAY(buffer + 0x04);
		
		UCharPtr uptr(new unsigned char[entries * entry_size]);
		unsigned char* raw = uptr.get();
		
		if(!::chm_retrieve_object(_chmFile, &ui, raw, 8, 
					  entries * entry_size))
			return false;

		if(::chm_resolve_object(_chmFile, "/#STRINGS", &ui) != 
		   CHM_RESOLVE_SUCCESS)
			return false;

		for(uint32_t i = 0; i < entries; ++i) {

			uint32_t offset = i * entry_size;

			uint32_t off_title = 
				UINT32ARRAY(raw + offset + 0x14);

			uint32_t off_home = 
				UINT32ARRAY(raw + offset + 0x68);

			uint32_t off_hhc = 
				UINT32ARRAY(raw + offset + 0x60);
			
			uint32_t off_hhk = 
				UINT32ARRAY(raw + offset + 0x64);

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


bool CHMFile::InfoFromSystem()
{
	unsigned char buffer[BUF_SIZE];
	chmUnitInfo ui;
	
	int index = 0;
	unsigned char* cursor = NULL;
	uint16_t value = 0;
	uint32_t lcid = 0;

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
		if(index > size - 1 - (long)sizeof(uint16_t))
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

		case 4: // LCID stuff
			index += 2;
			cursor = buffer + index;

			lcid = UINT32ARRAY(buffer + index + 2);
			_enc = GetFontEncFromLCID(lcid);
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

			if(_enc != wxFONTENCODING_SYSTEM)
				break;
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
		fontEncoding = wxFONTENCODING_CP1251;
		break;
        case ARABIC_CHARSET:
		fontEncoding = wxFONTENCODING_CP1256; 
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


inline 
wxFontEncoding CHMFile::GetFontEncFromLCID(uint32_t lcid)
{
	wxFontEncoding fontEncoding;	
	uint32_t lid = lcid & 0xff;
	
	switch(lid) {
	case LANG_ARABIC:
	case LANG_FARSI:
	case LANG_URDU:
		fontEncoding = wxFONTENCODING_CP1256; 
		//fontEncoding = wxFONTENCODING_UTF8; 
		break;
	case LANG_AZERI:
	case LANG_BELARUSIAN:
	case LANG_BULGARIAN:
	case LANG_KAZAK:
	case LANG_KYRGYZ:
	case LANG_MONGOLIAN:
	case LANG_TATAR:
	case LANG_RUSSIAN:
	case LANG_UKRAINIAN:
	case LANG_UZBEK:
		fontEncoding = wxFONTENCODING_CP1251;
		break;
	case LANG_AFRIKAANS:
	case LANG_BASQUE:
	case LANG_CATALAN:
	case LANG_DANISH:
	case LANG_DUTCH:
	case LANG_ENGLISH:
	case LANG_FINNISH:
	case LANG_FRENCH:
	case LANG_GALICIAN:
	case LANG_GERMAN:
	case LANG_ICELANDIC:
	case LANG_INDONESIAN:
	case LANG_ITALIAN:
	case LANG_MALAY:
	case LANG_NORWEGIAN:
	case LANG_PORTUGUESE:
	case LANG_SPANISH:
	case LANG_SWAHILI:
	case LANG_SWEDISH:
		fontEncoding = wxFONTENCODING_CP1252;
		break;
	case LANG_GREEK:
		fontEncoding = wxFONTENCODING_CP1253;
		break;
	case LANG_HEBREW:
		fontEncoding = wxFONTENCODING_CP1255;
		break;
        case LANG_THAI:
		fontEncoding = wxFONTENCODING_CP874;
		break;
        case LANG_TURKISH:
		fontEncoding = wxFONTENCODING_CP1254;
		break;
        case LANG_CHINESE:
		if(lcid == 0x0804) // Chinese simplified
			fontEncoding = wxFONTENCODING_CP936;
		else // Chinese traditional
			fontEncoding = wxFONTENCODING_CP950;
		break;
        case LANG_KOREAN:
		fontEncoding = wxFONTENCODING_CP949;
		break;
        case LANG_JAPANESE:
		fontEncoding = wxFONTENCODING_CP932;
		break;
	case LANG_ALBANIAN:
	case LANG_CROATIAN:
	case LANG_CZECH:
	case LANG_HUNGARIAN:
	case LANG_POLISH:
	case LANG_ROMANIAN:
	case LANG_SLOVAK:
	case LANG_SLOVENIAN:
		fontEncoding = wxFONTENCODING_CP1250;
		break;
	case LANG_NEUTRAL:
	default:
		fontEncoding = wxFONTENCODING_SYSTEM;
		break;
	}

	return fontEncoding;
}


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

