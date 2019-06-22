/*
  Copyright (C) 2003 - 2019  Razvan Cojocaru <rzvncj@gmail.com>
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

#include <assert.h>
#include <chmfile.h>
#include <chmlistctrl.h>
#include <hhcparser.h>
#include <wx/defs.h>
#include <wx/fontmap.h>
#include <wx/progdlg.h>
#include <wx/strconv.h>
#include <wx/treectrl.h>
#include <wx/wx.h>
#include <wxstringutils.h>

namespace {

//! Maximum allowed number of search-returned items.
constexpr size_t MAX_SEARCH_RESULTS {512};

// Big-enough buffer size for use with various routines.
constexpr size_t BUF_SIZE {4096};

// Thanks to Vadim Zeitlin.
constexpr int ANSI_CHARSET {0};
constexpr int SHIFTJIS_CHARSET {128};
constexpr int HANGUL_CHARSET {129};
constexpr int GB2312_CHARSET {134};
constexpr int CHINESEBIG5_CHARSET {136};
constexpr int OEM_CHARSET {255};
constexpr int HEBREW_CHARSET {177};
constexpr int ARABIC_CHARSET {178};
constexpr int GREEK_CHARSET {161};
constexpr int TURKISH_CHARSET {162};
constexpr int THAI_CHARSET {222};
constexpr int EASTEUROPE_CHARSET {238};
constexpr int RUSSIAN_CHARSET {204};
constexpr int BALTIC_CHARSET {186};

// Hello, Microsoft
constexpr uint32_t LANG_NEUTRAL {0x00};
constexpr uint32_t LANG_ARABIC {0x01};
constexpr uint32_t LANG_BULGARIAN {0x02};
constexpr uint32_t LANG_CATALAN {0x03};
constexpr uint32_t LANG_CHINESE {0x04};
constexpr uint32_t LANG_CZECH {0x05};
constexpr uint32_t LANG_DANISH {0x06};
constexpr uint32_t LANG_GERMAN {0x07};
constexpr uint32_t LANG_GREEK {0x08};
constexpr uint32_t LANG_ENGLISH {0x09};
constexpr uint32_t LANG_SPANISH {0x0a};
constexpr uint32_t LANG_FINNISH {0x0b};
constexpr uint32_t LANG_FRENCH {0x0c};
constexpr uint32_t LANG_HEBREW {0x0d};
constexpr uint32_t LANG_HUNGARIAN {0x0e};
constexpr uint32_t LANG_ICELANDIC {0x0f};
constexpr uint32_t LANG_ITALIAN {0x10};
constexpr uint32_t LANG_JAPANESE {0x11};
constexpr uint32_t LANG_KOREAN {0x12};
constexpr uint32_t LANG_DUTCH {0x13};
constexpr uint32_t LANG_NORWEGIAN {0x14};
constexpr uint32_t LANG_POLISH {0x15};
constexpr uint32_t LANG_PORTUGUESE {0x16};
constexpr uint32_t LANG_ROMANIAN {0x18};
constexpr uint32_t LANG_RUSSIAN {0x19};
constexpr uint32_t LANG_CROATIAN {0x1a};
constexpr uint32_t LANG_SLOVAK {0x1b};
constexpr uint32_t LANG_ALBANIAN {0x1c};
constexpr uint32_t LANG_SWEDISH {0x1d};
constexpr uint32_t LANG_THAI {0x1e};
constexpr uint32_t LANG_TURKISH {0x1f};
constexpr uint32_t LANG_URDU {0x20};
constexpr uint32_t LANG_INDONESIAN {0x21};
constexpr uint32_t LANG_UKRAINIAN {0x22};
constexpr uint32_t LANG_BELARUSIAN {0x23};
constexpr uint32_t LANG_SLOVENIAN {0x24};
constexpr uint32_t LANG_FARSI {0x29};
constexpr uint32_t LANG_AZERI {0x2c};
constexpr uint32_t LANG_BASQUE {0x2d};
constexpr uint32_t LANG_AFRIKAANS {0x36};
constexpr uint32_t LANG_MALAY {0x3e};
constexpr uint32_t LANG_KAZAK {0x3f};
constexpr uint32_t LANG_KYRGYZ {0x40};
constexpr uint32_t LANG_SWAHILI {0x41};
constexpr uint32_t LANG_UZBEK {0x43};
constexpr uint32_t LANG_TATAR {0x44};
constexpr uint32_t LANG_MONGOLIAN {0x50};
constexpr uint32_t LANG_GALICIAN {0x56};

#define FIXENDIAN16(x) (x = wxUINT16_SWAP_ON_BE(x))
#define FIXENDIAN32(x) (x = wxUINT32_SWAP_ON_BE(x))

inline uint16_t UINT16_FROM_ARRAY(const unsigned char* x)
{
    return x[0] | (static_cast<uint16_t>(x[1]) << 8);
}

inline uint32_t UINT32_FROM_ARRAY(const unsigned char* x)
{
    return UINT16_FROM_ARRAY(x) | (static_cast<uint32_t>(x[2]) << 16) | (static_cast<uint32_t>(x[3]) << 24);
}

#define INT32ARRAY(x) static_cast<int32_t>(UINT32_FROM_ARRAY(x))
#define INT16ARRAY(x) static_cast<int16_t>(UINT16_FROM_ARRAY(x))

inline uint64_t be_encint(unsigned char* buffer, size_t& length)
{
    uint64_t result {0};
    auto     shift = 0;

    length = 0;

    do {
        result |= ((*buffer) & 0x7f) << shift;
        shift += 7;
        ++length;

    } while (*(buffer++) & 0x80);

    return result;
}

/*
   Finds the first unset bit in memory. Returns the number of set bits found.
   Returns -1 if the buffer runs out before we find an unset bit.
*/
inline int ffus(unsigned char* byte, int* bit, size_t& length)
{
    auto bits = 0;

    length = 0;

    while (*byte & (1 << *bit)) {
        if (*bit)
            --(*bit);
        else {
            ++byte;
            ++length;
            *bit = 7;
        }
        ++bits;
    }

    if (*bit)
        --(*bit);
    else {
        ++length;
        *bit = 7;
    }

    return bits;
}

inline uint64_t sr_int(unsigned char* byte, int* bit, unsigned char s, unsigned char r, size_t& length)
{
    uint64_t ret {0};
    size_t   fflen;

    length = 0;

    if (!bit || *bit > 7 || s != 2)
        return ~static_cast<uint64_t>(0);

    auto count = ffus(byte, bit, fflen);
    length += fflen;
    byte += length;

    int  n_bits = r + (count ? count - 1 : 0);
    auto n      = n_bits;

    while (n > 0) {
        auto num_bits = n > *bit ? *bit : n - 1;
        auto base     = n > *bit ? 0 : *bit - (n - 1);
        auto mask     = static_cast<unsigned char>((0xff >> (7 - num_bits)) << base);
        ret           = (ret << (num_bits + 1)) | static_cast<uint64_t>((*byte & mask) >> base);

        if (n > *bit) {
            ++byte;
            ++length;
            n -= *bit + 1;
            *bit = 7;
        } else {
            *bit -= n;
            n = 0;
        }
    }

    if (count)
        ret |= static_cast<uint64_t>(1) << n_bits;

    return ret;
}

} // end of anonymous namespace

CHMFile::CHMFile(const wxString& archiveName)
{
    LoadCHM(archiveName);
}

CHMFile::~CHMFile()
{
    CloseCHM();
}

bool CHMFile::LoadCHM(const wxString& archiveName)
{
    CloseCHM();

    _chmFile = chm_open(static_cast<const char*>(archiveName.mb_str()));

    if (!_chmFile)
        return false;

    _enc      = wxFONTENCODING_SYSTEM;
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
    if (!_chmFile)
        return;

    chm_close(_chmFile);
    _chmFile = nullptr;

    _cidMap.clear();
    _home     = wxT("/");
    _filename = _home = _topicsFile = _indexFile = _title = _font = wxEmptyString;
}

#define MSG_RETR_TOC _("Retrieving table of contents..")
#define MSG_RETR_IDX _("Retrieving index..")
#define EMPTY_INDEX _("Untitled in index")

bool CHMFile::BinaryTOC(wxTreeCtrl& toBuild)
{
    chmUnitInfo ti_ui, ts_ui, st_ui, ut_ui, us_ui;

    if (chm_resolve_object(_chmFile, "/#TOCIDX", &ti_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#TOPICS", &ts_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#STRINGS", &st_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLTBL", &ut_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLSTR", &us_ui) != CHM_RESOLVE_SUCCESS)
        return false; // failed to find internal files

    if (ti_ui.length < 4)
        return false;

    UCharVector topidx(ti_ui.length), topics(ts_ui.length), strings(st_ui.length), urltbl(ut_ui.length),
        urlstr(us_ui.length);

    if (chm_retrieve_object(_chmFile, &ti_ui, &topidx[0], 0, ti_ui.length) != static_cast<int64_t>(ti_ui.length)
        || chm_retrieve_object(_chmFile, &ts_ui, &topics[0], 0, ts_ui.length) != static_cast<int64_t>(ts_ui.length)
        || chm_retrieve_object(_chmFile, &st_ui, &strings[0], 0, st_ui.length) != static_cast<int64_t>(st_ui.length)
        || chm_retrieve_object(_chmFile, &ut_ui, &urltbl[0], 0, ut_ui.length) != static_cast<int64_t>(ut_ui.length)
        || chm_retrieve_object(_chmFile, &us_ui, &urlstr[0], 0, us_ui.length) != static_cast<int64_t>(us_ui.length))
        return false;

    auto off = UINT32_FROM_ARRAY(&topidx[0]);
    RecurseLoadBTOC(topidx, topics, strings, urltbl, urlstr, off, toBuild, 1);

    return true;
}

void CHMFile::RecurseLoadBTOC(UCharVector& topidx, UCharVector& topics, UCharVector& strings, UCharVector& urltbl,
                              UCharVector& urlstr, uint32_t offset, wxTreeCtrl& toBuild, int level)
{
    while (offset) {
        if (topidx.size() < offset + 20)
            return;

        auto flags = UINT32_FROM_ARRAY(&topidx[offset + 4]);
        auto index = UINT32_FROM_ARRAY(&topidx[offset + 8]);

        if ((flags & 0x4) || (flags & 0x8)) // book or local
            if (!GetItem(topics, strings, urltbl, urlstr, index, &toBuild, nullptr, wxEmptyString, level,
                         (flags & 0x8) == 0))
                return;

        if (flags & 0x4) { // book
            if (topidx.size() < offset + 24)
                return;

            auto child = UINT32_FROM_ARRAY(&topidx[offset + 20]);

            if (child)
                RecurseLoadBTOC(topidx, topics, strings, urltbl, urlstr, child, toBuild, level + 1);
        }

        offset = UINT32_FROM_ARRAY(&topidx[offset + 0x10]);
    }
}

bool CHMFile::GetItem(UCharVector& topics, UCharVector& strings, UCharVector& urltbl, UCharVector& urlstr,
                      uint32_t index, wxTreeCtrl* tree, CHMListCtrl* list, const wxString& idxName, int level,
                      bool local)
{
    static wxTreeItemId   parents[TREE_BUF_SIZE];
    static auto           calls      = 0;
    static constexpr auto YIELD_TIME = 256;

    ++calls;

    if (calls % YIELD_TIME) {
        calls = 0;
        wxYield();
    }

    if (tree)
        parents[0] = tree->GetRootItem();

    std::string name, value;

    if (local) {
        if (strings.size() < index + 1)
            return false;

        name = reinterpret_cast<char*>(&strings[index]);

    } else {
        if (topics.size() < (index * 16) + 12)
            return false;

        auto offset = UINT32_FROM_ARRAY(&topics[index * 16 + 4]);
        auto test   = static_cast<int32_t>(offset);

        if (strings.size() < offset + 1 || test == -1)
            return false;

        if (!list)
            name = reinterpret_cast<char*>(&strings[offset]);

        // #URLTBL index
        offset = UINT32_FROM_ARRAY(&topics[index * 16 + 8]);

        if (urltbl.size() < offset + 12)
            return false;

        offset = UINT32_FROM_ARRAY(&urltbl[offset + 8]);

        if (urlstr.size() < offset)
            return false;

        value = reinterpret_cast<char*>(&urlstr[offset + 8]);
    }

    if (!value.empty() && value[0] != '/')
        value = "/" + value;

    auto tvalue = CURRENT_CHAR_STRING(value.c_str());

    if (tree && !name.empty()) {
        auto parentIndex = level ? level - 1 : 0;
        auto tname       = translateEncoding(CURRENT_CHAR_STRING(name.c_str()), _enc);

        parents[level] = tree->AppendItem(parents[parentIndex], tname, 2, 2, new URLTreeItem(tvalue));

        if (level && tree->GetItemImage(parents[parentIndex]) != 0) {
            tree->SetItemImage(parents[parentIndex], 0, wxTreeItemIcon_Normal);
            tree->SetItemImage(parents[parentIndex], 0, wxTreeItemIcon_Selected);
            tree->SetItemImage(parents[parentIndex], 1, wxTreeItemIcon_Expanded);
        }
    }

    if (list) {
        auto tname = idxName;
        if (!value.empty() && tname.IsEmpty())
            tname = EMPTY_INDEX;

        list->AddPairItem(tname, tvalue);
    }

    return true;
}

bool CHMFile::GetTopicsTree(wxTreeCtrl& toBuild)
{
    chmUnitInfo ui;
    char        buffer[BUF_SIZE] {};
    size_t      ret {BUF_SIZE - 1}, curr {0};

    toBuild.Freeze();
    bool btoc = BinaryTOC(toBuild);
    toBuild.Thaw();

    if (btoc)
        return true;

    // Fall back to parsing the HTML TOC file, if that's in the archive
    if (_topicsFile.IsEmpty() || !ResolveObject(_topicsFile, &ui))
        return false;

    toBuild.Freeze();

    HHCParser p(_enc, &toBuild, nullptr);

    do {
        ret         = RetrieveObject(&ui, reinterpret_cast<unsigned char*>(buffer), curr, BUF_SIZE - 1);
        buffer[ret] = '\0';

        p.parse(buffer);
        curr += ret;
    } while (ret == BUF_SIZE - 1);

    toBuild.Thaw();

    return true;
}

// This function is too long: prime candidate for refactoring someday
bool CHMFile::BinaryIndex(CHMListCtrl& toBuild, const wxCSConv& cv)
{
    chmUnitInfo bt_ui, ts_ui, st_ui, ut_ui, us_ui;
    auto        items = 0UL;

    if (chm_resolve_object(_chmFile, "/$WWKeywordLinks/BTree", &bt_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#TOPICS", &ts_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#STRINGS", &st_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLTBL", &ut_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLSTR", &us_ui) != CHM_RESOLVE_SUCCESS)
        return false; // failed to find internal files

    UCharVector btree(bt_ui.length), topics(ts_ui.length), strings(st_ui.length), urltbl(ut_ui.length),
        urlstr(us_ui.length);

    if (chm_retrieve_object(_chmFile, &bt_ui, &btree[0], 0, bt_ui.length) != (int64_t)bt_ui.length
        || chm_retrieve_object(_chmFile, &ts_ui, &topics[0], 0, ts_ui.length) != (int64_t)ts_ui.length
        || chm_retrieve_object(_chmFile, &st_ui, &strings[0], 0, st_ui.length) != (int64_t)st_ui.length
        || chm_retrieve_object(_chmFile, &ut_ui, &urltbl[0], 0, ut_ui.length) != (int64_t)ut_ui.length
        || chm_retrieve_object(_chmFile, &us_ui, &urlstr[0], 0, us_ui.length) != (int64_t)us_ui.length)
        return false;

    if (bt_ui.length < 0x4c + 12)
        return false;

    unsigned long      offset {0x4c};
    int32_t            next {-1};
    constexpr uint16_t BLOCK_SIZE {2048};

    do {
        if (bt_ui.length < offset + 12)
            return items != 0; // end of buffer

        auto freeSpace = UINT16_FROM_ARRAY(&btree[offset]);
        next           = INT32ARRAY(&btree[offset + 8]);
        auto spaceLeft = BLOCK_SIZE - 12;
        offset += 12;

        while (spaceLeft > freeSpace) {
            uint16_t tmp {0};
            wxString name;

            do { // accumulate the index name
                if (bt_ui.length < offset + sizeof(uint16_t))
                    return items != 0;

                tmp = UINT16_FROM_ARRAY(&btree[offset]);
                offset += sizeof(uint16_t);
                spaceLeft -= sizeof(uint16_t);

                name.Append(charForCode(tmp, cv, true));
            } while (tmp != 0);

            if (bt_ui.length < offset + 16)
                return items != 0;

            auto seeAlso   = UINT16_FROM_ARRAY(&btree[offset]);
            auto numTopics = UINT32_FROM_ARRAY(&btree[offset + 0xc]);

            offset += 16;
            spaceLeft -= 16;

            if (seeAlso) {
                do { // get over the Unicode string
                    if (bt_ui.length < offset + sizeof(uint16_t))
                        return items != 0;

                    tmp = UINT16_FROM_ARRAY(&btree[offset]);
                    offset += sizeof(uint16_t);
                    spaceLeft -= sizeof(uint16_t);
                } while (tmp != 0);

            } else {
                for (uint32_t i = 0; i < numTopics && spaceLeft > freeSpace; ++i) {
                    if (bt_ui.length < offset + sizeof(uint32_t))
                        return items != 0;

                    auto index = UINT32_FROM_ARRAY(&btree[offset]);

                    GetItem(topics, strings, urltbl, urlstr, index, nullptr, &toBuild, name, 0, false);
                    ++items;

                    offset += sizeof(uint32_t);
                    spaceLeft -= sizeof(uint32_t);
                }
            }

            if (bt_ui.length < offset + 8)
                return items != 0;

            offset += 8;
            spaceLeft -= 8;
        }

        offset += spaceLeft;

    } while (next != -1);

    return items != 0;
}

bool CHMFile::GetIndex(CHMListCtrl& toBuild)
{
    chmUnitInfo ui;
    char        buffer[BUF_SIZE] {};
    size_t      ret {BUF_SIZE - 1}, curr {0};

    std::unique_ptr<wxCSConv> cvPtr = createCSConvPtr(_enc);

    toBuild.Freeze();
    bool bindex = BinaryIndex(toBuild, *cvPtr);
    toBuild.Thaw();

    if (bindex)
        return true;

    if (_indexFile.IsEmpty() || !ResolveObject(_indexFile, &ui))
        return false;

    toBuild.Freeze();

    HHCParser p(_enc, nullptr, &toBuild);

    do {
        ret         = RetrieveObject(&ui, reinterpret_cast<unsigned char*>(buffer), curr, BUF_SIZE - 1);
        buffer[ret] = '\0';

        p.parse(buffer);
        curr += ret;
    } while (ret == BUF_SIZE - 1);

    toBuild.Thaw();

    return true;
}

bool CHMFile::LoadContextIDs()
{
    chmUnitInfo ivb_ui, strs_ui;

    _cidMap.clear();

    // make sure what we need is there.
    // #IVB has list of context ID's and #STRINGS offsets to file names.
    if (chm_resolve_object(_chmFile, "/#IVB", &ivb_ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#STRINGS", &strs_ui) != CHM_RESOLVE_SUCCESS)
        return false; // failed to find internal files

    UCharVector ivb_buf(ivb_ui.length);
    uint64_t    ivb_len {0};

    if ((ivb_len = chm_retrieve_object(_chmFile, &ivb_ui, &ivb_buf[0], 0, ivb_ui.length)) == 0)
        return false; // failed to retrieve data

    // always odd (DWORD + 2(n)*DWORD, so make even
    ivb_len = ivb_len / sizeof(uint32_t) - 1;

    if (ivb_len % 2 != 0)
        return false; // we retrieved unexpected data from the file.

    std::vector<uint32_t> ivbs(ivb_len);
    auto                  j = 4; // offset to exclude first DWORD

    // convert our DWORDs to numbers
    for (unsigned int i = 0; i < ivb_len; ++i) {
        ivbs[i] = UINT32_FROM_ARRAY(&ivb_buf[j]);
        j += 4; // step to the next DWORD
    }

    UCharVector strs_buf(strs_ui.length);

    if (chm_retrieve_object(_chmFile, &strs_ui, &strs_buf[0], 0, strs_ui.length) == 0)
        return false; // failed to retrieve data

    for (unsigned int i = 0; i < ivb_len; i += 2)
        // context-IDs as KEY, fileName from #STRINGS as VALUE
        _cidMap[ivbs[i]] = CURRENT_CHAR_STRING(&strs_buf[ivbs[i + 1]]);

    // everything went well!
    return true;
}

bool CHMFile::IsValidCID(int contextID)
{
    return _cidMap.find(contextID) != _cidMap.end();
}

wxString CHMFile::GetPageByCID(int contextID)
{
    auto itr = _cidMap.find(contextID);

    if (itr == _cidMap.end())
        return wxT("/");

    return wxT("/") + itr->second;
}

bool CHMFile::IndexSearch(const wxString& text, bool wholeWords, bool titlesOnly, CHMSearchResults& results)
{
    auto partial = false;

    if (text.IsEmpty())
        return false;

    chmUnitInfo ui, uitopics, uiurltbl, uistrings, uiurlstr;
    if (chm_resolve_object(_chmFile, "/$FIftiMain", &ui) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#TOPICS", &uitopics) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#STRINGS", &uistrings) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLTBL", &uiurltbl) != CHM_RESOLVE_SUCCESS
        || chm_resolve_object(_chmFile, "/#URLSTR", &uiurlstr) != CHM_RESOLVE_SUCCESS)
        return false;

    constexpr size_t FTS_HEADER_LEN {0x32};
    unsigned char    header[FTS_HEADER_LEN];

    if (chm_retrieve_object(_chmFile, &ui, header, 0, FTS_HEADER_LEN) == 0)
        return false;

    auto doc_index_s = header[0x1E], doc_index_r = header[0x1F], code_count_s = header[0x20],
         code_count_r = header[0x21], loc_codes_s = header[0x22], loc_codes_r = header[0x23];

    if (doc_index_s != 2 || code_count_s != 2 || loc_codes_s != 2)
        // Don't know how to use values other than 2 yet. Maybe next chmspec.
        return false;

    auto cursor32    = header + 0x14;
    auto node_offset = UINT32_FROM_ARRAY(cursor32);

    cursor32      = header + 0x2e;
    auto node_len = UINT32_FROM_ARRAY(cursor32);

    auto cursor16   = header + 0x18;
    auto tree_depth = UINT16_FROM_ARRAY(cursor16);

    wxString    word;
    UCharVector buffer(node_len);

    node_offset = GetLeafNodeOffset(text, node_offset, node_len, tree_depth, &ui);

    if (!node_offset)
        return false;

    do {
        // got a leaf node here.
        if (chm_retrieve_object(_chmFile, &ui, &buffer[0], node_offset, node_len) == 0)
            return false;

        cursor16        = &buffer[6];
        auto free_space = UINT16_FROM_ARRAY(cursor16);

        auto i = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t);

        while (i < node_len - free_space) {
            auto word_len = buffer[i];
            auto pos      = buffer[i + 1];

            std::vector<char> wrd_buf(word_len);

            memcpy(&wrd_buf[0], &buffer[i + 2], word_len - 1);
            wrd_buf[word_len - 1] = 0;

            if (pos == 0)
                word = CURRENT_CHAR_STRING(&wrd_buf[0]);
            else
                word = word.Mid(0, pos) + CURRENT_CHAR_STRING(&wrd_buf[0]);

            i += 2 + word_len;

            auto   title = buffer[i - 1];
            size_t encsz;

            auto wlc_count = be_encint(&buffer[i], encsz);
            i += encsz;

            cursor32        = &buffer[i];
            auto wlc_offset = UINT32_FROM_ARRAY(cursor32);

            i += sizeof(uint32_t) + sizeof(uint16_t);
            auto wlc_size = be_encint(&buffer[i], encsz);
            i += encsz;

            cursor32    = &buffer[0];
            node_offset = UINT32_FROM_ARRAY(cursor32);

            if (!title && titlesOnly)
                continue;

            if (wholeWords && !text.CmpNoCase(word))
                return ProcessWLC(wlc_count, wlc_size, wlc_offset, doc_index_s, doc_index_r, code_count_s, code_count_r,
                                  loc_codes_s, loc_codes_r, &ui, &uiurltbl, &uistrings, &uitopics, &uiurlstr, results);

            if (!wholeWords) {
                if (word.StartsWith(text.c_str())) {
                    partial = true;
                    ProcessWLC(wlc_count, wlc_size, wlc_offset, doc_index_s, doc_index_r, code_count_s, code_count_r,
                               loc_codes_s, loc_codes_r, &ui, &uiurltbl, &uistrings, &uitopics, &uiurlstr, results);

                } else if (text.CmpNoCase(word.Mid(0, text.Length())) < -1)
                    break;
            }

            if (results.size() >= MAX_SEARCH_RESULTS)
                break;
        }
    } while (!wholeWords && word.StartsWith(text.c_str()) && node_offset);

    return partial;
}

bool CHMFile::ResolveObject(const wxString& fileName, chmUnitInfo* ui)
{
    return _chmFile
        && chm_resolve_object(_chmFile, static_cast<const char*>(fileName.mb_str()), ui) == CHM_RESOLVE_SUCCESS;
}

size_t CHMFile::RetrieveObject(chmUnitInfo* ui, unsigned char* buffer, off_t fileOffset, size_t bufferSize)
{
    return chm_retrieve_object(_chmFile, ui, buffer, fileOffset, bufferSize);
}

bool CHMFile::GetArchiveInfo()
{
    // Order counts, parse #SYSTEM before #WINDOWS (thanks to Kuang-che Wu for pointing that out)
    auto rets = InfoFromSystem();
    auto retw = InfoFromWindows();

    return retw || rets;
}

uint32_t CHMFile::GetLeafNodeOffset(const wxString& text, uint32_t initialOffset, uint32_t buffSize, uint16_t treeDepth,
                                    chmUnitInfo* ui)
{
    uint32_t test_offset {0};
    uint32_t i {sizeof(uint16_t)};
    wxString word;

    if (!buffSize)
        return 0;

    UCharVector buffer(buffSize);

    while (--treeDepth) {
        if (initialOffset == test_offset)
            return 0;

        test_offset = initialOffset;
        if (chm_retrieve_object(_chmFile, ui, &buffer[0], initialOffset, buffSize) == 0)
            return 0;

        auto cursor16   = &buffer[0];
        auto free_space = UINT16_FROM_ARRAY(cursor16);

        while (i < buffSize - free_space) {
            auto word_len = buffer[i];
            auto pos      = buffer[i + 1];

            std::vector<char> wrd_buf(word_len);

            memcpy(&wrd_buf[0], &buffer[i + 2], word_len - 1);
            wrd_buf[word_len - 1] = 0;

            if (pos == 0)
                word = CURRENT_CHAR_STRING(&wrd_buf[0]);
            else
                word = word.Mid(0, pos) + CURRENT_CHAR_STRING(&wrd_buf[0]);

            if (text.CmpNoCase(word) <= 0) {
                auto cursor32 = &buffer[i + word_len + 1];
                initialOffset = UINT32_FROM_ARRAY(cursor32);
                break;
            }

            i += word_len + sizeof(unsigned char) + sizeof(uint32_t) + sizeof(uint16_t);
        }
    }

    return initialOffset == test_offset ? 0 : initialOffset;
}

bool CHMFile::ProcessWLC(uint64_t wlc_count, uint64_t wlc_size, uint32_t wlc_offset, unsigned char ds, unsigned char dr,
                         unsigned char cs, unsigned char cr, unsigned char ls, unsigned char lr, chmUnitInfo* uimain,
                         chmUnitInfo* uitbl, chmUnitInfo* uistrings, chmUnitInfo* topics, chmUnitInfo* urlstr,
                         CHMSearchResults& results)
{
    auto        wlc_bit = 7;
    uint64_t    index {0};
    size_t      length, off {0};
    UCharVector buffer(wlc_size);

    constexpr size_t TOPICS_ENTRY_LEN {16};
    unsigned char    entry[TOPICS_ENTRY_LEN];

    constexpr size_t COMMON_BUF_LEN {1025};
    unsigned char    combuf[COMMON_BUF_LEN];

    if (chm_retrieve_object(_chmFile, uimain, &buffer[0], wlc_offset, wlc_size) == 0)
        return false;

    for (uint64_t i = 0; i < wlc_count; ++i) {
        if (wlc_bit != 7) {
            ++off;
            wlc_bit = 7;
        }

        index += sr_int(&buffer[off], &wlc_bit, ds, dr, length);
        off += length;

        if (chm_retrieve_object(_chmFile, topics, entry, index * 16, TOPICS_ENTRY_LEN) == 0)
            return false;

        auto cursor32              = entry + 4;
        combuf[COMMON_BUF_LEN - 1] = 0;
        auto stroff                = UINT32_FROM_ARRAY(cursor32);

        wxString topic;

        if (chm_retrieve_object(_chmFile, uistrings, combuf, stroff, COMMON_BUF_LEN - 1) == 0)
            topic = EMPTY_INDEX;
        else {
            combuf[COMMON_BUF_LEN - 1] = 0;

#if wxUSE_UNICODE
            if (_enc == wxFONTENCODING_SYSTEM)
#endif
                topic = CURRENT_CHAR_STRING(combuf);
#if wxUSE_UNICODE
            else {
                std::unique_ptr<wxCSConv> cvPtr = createCSConvPtr(_enc);
                topic                           = wxString(reinterpret_cast<const char*>(combuf), *cvPtr);
            }
#endif
        }

        cursor32    = entry + 8;
        auto urloff = UINT32_FROM_ARRAY(cursor32);

        if (chm_retrieve_object(_chmFile, uitbl, combuf, urloff, 12) == 0)
            return false;

        cursor32 = combuf + 8;
        urloff   = UINT32_FROM_ARRAY(cursor32);

        if (chm_retrieve_object(_chmFile, urlstr, combuf, urloff + 8, COMMON_BUF_LEN - 1) == 0)
            return false;

        combuf[COMMON_BUF_LEN - 1] = 0;

        auto url = CURRENT_CHAR_STRING(combuf);

        if (!url.IsEmpty() && !topic.IsEmpty()) {
            if (results.size() >= MAX_SEARCH_RESULTS)
                return true;
            results[url] = topic;
        }

        auto count = sr_int(&buffer[off], &wlc_bit, cs, cr, length);
        off += length;

        for (uint64_t j = 0; j < count; ++j) {
            sr_int(&buffer[off], &wlc_bit, ls, lr, length);
            off += length;
        }
    }

    return true;
}

bool CHMFile::InfoFromWindows()
{
    constexpr size_t WIN_HEADER_LEN {0x08};
    chmUnitInfo      ui;

    if (chm_resolve_object(_chmFile, "/#WINDOWS", &ui) == CHM_RESOLVE_SUCCESS) {
        unsigned char buffer[BUF_SIZE];
        auto          size = 0L;

        if (!chm_retrieve_object(_chmFile, &ui, buffer, 0, WIN_HEADER_LEN))
            return false;

        auto entries    = UINT32_FROM_ARRAY(buffer);
        auto entry_size = UINT32_FROM_ARRAY(buffer + 0x04);

        UCharVector uptr(entries * entry_size);
        auto        raw = &uptr[0];

        if (!chm_retrieve_object(_chmFile, &ui, raw, 8, entries * entry_size))
            return false;

        if (chm_resolve_object(_chmFile, "/#STRINGS", &ui) != CHM_RESOLVE_SUCCESS)
            return false;

        for (uint32_t i = 0; i < entries; ++i) {
            uint32_t offset {i * entry_size};
            auto     off_title = UINT32_FROM_ARRAY(raw + offset + 0x14);
            auto     off_home  = UINT32_FROM_ARRAY(raw + offset + 0x68);
            auto     off_hhc   = UINT32_FROM_ARRAY(raw + offset + 0x60);
            auto     off_hhk   = UINT32_FROM_ARRAY(raw + offset + 0x64);
            auto     factor    = off_title / 4096;

            if (size == 0)
                size = chm_retrieve_object(_chmFile, &ui, buffer, factor * 4096, BUF_SIZE);

            if (size && off_title)
                _title = CURRENT_CHAR_STRING(buffer + off_title % 4096);

            if (factor != off_home / 4096) {
                factor = off_home / 4096;
                size   = chm_retrieve_object(_chmFile, &ui, buffer, factor * 4096, BUF_SIZE);
            }

            if (size && off_home)
                _home = wxT("/") + CURRENT_CHAR_STRING(buffer + off_home % 4096);

            if (factor != off_hhc / 4096) {
                factor = off_hhc / 4096;
                size   = chm_retrieve_object(_chmFile, &ui, buffer, factor * 4096, BUF_SIZE);
            }

            if (size && off_hhc)
                _topicsFile = wxT("/") + CURRENT_CHAR_STRING(buffer + off_hhc % 4096);

            if (factor != off_hhk / 4096) {
                factor = off_hhk / 4096;
                size   = chm_retrieve_object(_chmFile, &ui, buffer, factor * 4096, BUF_SIZE);
            }

            if (size && off_hhk)
                _indexFile = wxT("/") + CURRENT_CHAR_STRING(buffer + off_hhk % 4096);
        }
    }

    return true;
}

bool CHMFile::InfoFromSystem()
{
    unsigned char buffer[BUF_SIZE] {};
    chmUnitInfo   ui;
    auto          index = 0;
    auto          size  = 0L;

    // Do we have the #SYSTEM file in the archive?
    if (chm_resolve_object(_chmFile, "/#SYSTEM", &ui) != CHM_RESOLVE_SUCCESS)
        return false;

    // Can we pull BUFF_SIZE bytes of the #SYSTEM file?
    if ((size = chm_retrieve_object(_chmFile, &ui, buffer, 4, BUF_SIZE)) == 0)
        return false;

    for (;;) {
        // This condition won't hold if I process anything except NUL-terminated strings!
        if (index > size - 1 - static_cast<long>(sizeof(uint16_t)))
            break;

        auto cursor = buffer + index;
        auto value  = UINT16_FROM_ARRAY(cursor);

        switch (value) {
        case 0:
            index += 2;
            cursor = buffer + index;

            if (_topicsFile.IsEmpty())
                _topicsFile = wxT("/") + CURRENT_CHAR_STRING(buffer + index + 2);
            break;

        case 1:
            index += 2;
            cursor = buffer + index;

            if (_indexFile.IsEmpty())
                _indexFile = wxT("/") + CURRENT_CHAR_STRING(buffer + index + 2);
            break;

        case 2:
            index += 2;
            cursor = buffer + index;

            if (_home.IsEmpty() || _home == wxT("/"))
                _home = wxT("/") + CURRENT_CHAR_STRING(buffer + index + 2);
            break;

        case 3:
            index += 2;
            cursor = buffer + index;

            if (_title.IsEmpty())
                _title = CURRENT_CHAR_STRING(buffer + index + 2);
            break;

        case 4: // LCID stuff
            index += 2;
            cursor = buffer + index;

            _enc = GetFontEncFromLCID(UINT32_FROM_ARRAY(buffer + index + 2));
            break;

        case 6:
            index += 2;
            cursor = buffer + index;

            if (_topicsFile.IsEmpty()) {
                wxString topicAttempt {wxT("/")}, tmp;
                topicAttempt += CURRENT_CHAR_STRING(buffer + index + 2);

                tmp = topicAttempt + wxT(".hhc");

                if (chm_resolve_object(_chmFile, tmp.mb_str(), &ui) == CHM_RESOLVE_SUCCESS)
                    _topicsFile = tmp;

                tmp = topicAttempt + wxT(".hhk");

                if (chm_resolve_object(_chmFile, tmp.mb_str(), &ui) == CHM_RESOLVE_SUCCESS)
                    _indexFile = tmp;
            }

            break;

        case 16: {
            auto cs = -1L;

            index += 2;
            cursor = buffer + index;

            _font = CURRENT_CHAR_STRING(buffer + index + 2);
            _font.AfterLast(wxT(',')).ToLong(&cs);

            if (_enc != wxFONTENCODING_SYSTEM)
                break;
            _enc = GetFontEncFromCharSet(cs);
            break;
        }

        default:
            index += 2;
            cursor = buffer + index;
        }

        value = UINT16_FROM_ARRAY(cursor);
        index += value + 2;
    }

    return true;
}

inline wxFontEncoding CHMFile::GetFontEncFromCharSet(int cs)
{
    switch (cs) {
    case ANSI_CHARSET:
        return wxFONTENCODING_ISO8859_1;
    case EASTEUROPE_CHARSET:
        return wxFONTENCODING_ISO8859_2;
    case BALTIC_CHARSET:
        return wxFONTENCODING_ISO8859_13;
    case RUSSIAN_CHARSET:
        return wxFONTENCODING_CP1251;
    case ARABIC_CHARSET:
        return wxFONTENCODING_CP1256;
    case GREEK_CHARSET:
        return wxFONTENCODING_ISO8859_7;
    case HEBREW_CHARSET:
        return wxFONTENCODING_ISO8859_8;
    case TURKISH_CHARSET:
        return wxFONTENCODING_ISO8859_9;
    case THAI_CHARSET:
        return wxFONTENCODING_ISO8859_11;
    case SHIFTJIS_CHARSET:
        return wxFONTENCODING_CP932;
    case GB2312_CHARSET:
        return wxFONTENCODING_CP936;
    case HANGUL_CHARSET:
        return wxFONTENCODING_CP949;
    case CHINESEBIG5_CHARSET:
        return wxFONTENCODING_CP950;
    case OEM_CHARSET:
        return wxFONTENCODING_CP437;
    default:
        // assume the system charset
        return wxFONTENCODING_SYSTEM;
    }
}

inline wxFontEncoding CHMFile::GetFontEncFromLCID(uint32_t lcid)
{
    switch (lcid & 0xff) {
    case LANG_ARABIC:
    case LANG_FARSI:
    case LANG_URDU:
        return wxFONTENCODING_CP1256;
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
        return wxFONTENCODING_CP1251;
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
        return wxFONTENCODING_CP1252;
    case LANG_GREEK:
        return wxFONTENCODING_CP1253;
    case LANG_HEBREW:
        return wxFONTENCODING_CP1255;
    case LANG_THAI:
        return wxFONTENCODING_CP874;
    case LANG_TURKISH:
        return wxFONTENCODING_CP1254;
    case LANG_CHINESE:
        if (lcid == 0x0804) // Chinese simplified
            return wxFONTENCODING_CP936;
        else // Chinese traditional
            return wxFONTENCODING_CP950;
    case LANG_KOREAN:
        return wxFONTENCODING_CP949;
    case LANG_JAPANESE:
        return wxFONTENCODING_CP932;
    case LANG_ALBANIAN:
    case LANG_CROATIAN:
    case LANG_CZECH:
    case LANG_HUNGARIAN:
    case LANG_POLISH:
    case LANG_ROMANIAN:
    case LANG_SLOVAK:
    case LANG_SLOVENIAN:
        return wxFONTENCODING_CP1250;
    case LANG_NEUTRAL:
    default:
        return wxFONTENCODING_SYSTEM;
    }
}
