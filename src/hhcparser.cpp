/*
  Copyright (C) 2003 - 2026  Razvan Cojocaru <razvanc@mailbox.org>

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

#include <chmlistctrl.h>
#include <ctype.h>
#include <hhcparser.h>
#include <map>
#include <stdlib.h>
#include <wx/wx.h>
#include <wxstringutils.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace {

struct HTMLChar {
    const wxChar* name;
    unsigned      code;
};

extern "C" int HTMLCharCompare(const void* key, const void* item)
{
    return wxStrcmp(static_cast<const wxChar*>(key), static_cast<const HTMLChar*>(item)->name);
}

HTMLChar substitutions[] = {
    {wxT("AElig"), 198},    {wxT("Aacute"), 193},  {wxT("Acirc"), 194},   {wxT("Agrave"), 192},  {wxT("Alpha"), 913},
    {wxT("Aring"), 197},    {wxT("Atilde"), 195},  {wxT("Auml"), 196},    {wxT("Beta"), 914},    {wxT("Ccedil"), 199},
    {wxT("Chi"), 935},      {wxT("Dagger"), 8225}, {wxT("Delta"), 916},   {wxT("Dstrok"), 208},  {wxT("ETH"), 208},
    {wxT("Eacute"), 201},   {wxT("Ecirc"), 202},   {wxT("Egrave"), 200},  {wxT("Epsilon"), 917}, {wxT("Eta"), 919},
    {wxT("Euml"), 203},     {wxT("Gamma"), 915},   {wxT("Iacute"), 205},  {wxT("Icirc"), 206},   {wxT("Igrave"), 204},
    {wxT("Iota"), 921},     {wxT("Iuml"), 207},    {wxT("Kappa"), 922},   {wxT("Lambda"), 923},  {wxT("Mu"), 924},
    {wxT("Ntilde"), 209},   {wxT("Nu"), 925},      {wxT("OElig"), 338},   {wxT("Oacute"), 211},  {wxT("Ocirc"), 212},
    {wxT("Ograve"), 210},   {wxT("Omega"), 937},   {wxT("Omicron"), 927}, {wxT("Oslash"), 216},  {wxT("Otilde"), 213},
    {wxT("Ouml"), 214},     {wxT("Phi"), 934},     {wxT("Pi"), 928},      {wxT("Prime"), 8243},  {wxT("Psi"), 936},
    {wxT("Rho"), 929},      {wxT("Scaron"), 352},  {wxT("Sigma"), 931},   {wxT("THORN"), 222},   {wxT("Tau"), 932},
    {wxT("Theta"), 920},    {wxT("Uacute"), 218},  {wxT("Ucirc"), 219},   {wxT("Ugrave"), 217},  {wxT("Upsilon"), 933},
    {wxT("Uuml"), 220},     {wxT("Xi"), 926},      {wxT("Yacute"), 221},  {wxT("Yuml"), 376},    {wxT("Zeta"), 918},
    {wxT("aacute"), 225},   {wxT("acirc"), 226},   {wxT("acute"), 180},   {wxT("aelig"), 230},   {wxT("agrave"), 224},
    {wxT("alefsym"), 8501}, {wxT("alpha"), 945},   {wxT("amp"), 38},      {wxT("and"), 8743},    {wxT("ang"), 8736},
    {wxT("aring"), 229},    {wxT("asymp"), 8776},  {wxT("atilde"), 227},  {wxT("auml"), 228},    {wxT("bdquo"), 8222},
    {wxT("beta"), 946},     {wxT("brvbar"), 166},  {wxT("bull"), 8226},   {wxT("cap"), 8745},    {wxT("ccedil"), 231},
    {wxT("cedil"), 184},    {wxT("cent"), 162},    {wxT("chi"), 967},     {wxT("circ"), 710},    {wxT("clubs"), 9827},
    {wxT("cong"), 8773},    {wxT("copy"), 169},    {wxT("crarr"), 8629},  {wxT("cup"), 8746},    {wxT("curren"), 164},
    {wxT("dArr"), 8659},    {wxT("dagger"), 8224}, {wxT("darr"), 8595},   {wxT("deg"), 176},     {wxT("delta"), 948},
    {wxT("diams"), 9830},   {wxT("divide"), 247},  {wxT("eacute"), 233},  {wxT("ecirc"), 234},   {wxT("egrave"), 232},
    {wxT("empty"), 8709},   {wxT("emsp"), 8195},   {wxT("ensp"), 8194},   {wxT("epsilon"), 949}, {wxT("equiv"), 8801},
    {wxT("eta"), 951},      {wxT("eth"), 240},     {wxT("euml"), 235},    {wxT("euro"), 8364},   {wxT("exist"), 8707},
    {wxT("fnof"), 402},     {wxT("forall"), 8704}, {wxT("frac12"), 189},  {wxT("frac14"), 188},  {wxT("frac34"), 190},
    {wxT("frasl"), 8260},   {wxT("gamma"), 947},   {wxT("ge"), 8805},     {wxT("gt"), 62},       {wxT("hArr"), 8660},
    {wxT("harr"), 8596},    {wxT("hearts"), 9829}, {wxT("hellip"), 8230}, {wxT("iacute"), 237},  {wxT("icirc"), 238},
    {wxT("iexcl"), 161},    {wxT("igrave"), 236},  {wxT("image"), 8465},  {wxT("infin"), 8734},  {wxT("int"), 8747},
    {wxT("iota"), 953},     {wxT("iquest"), 191},  {wxT("isin"), 8712},   {wxT("iuml"), 239},    {wxT("kappa"), 954},
    {wxT("lArr"), 8656},    {wxT("lambda"), 955},  {wxT("lang"), 9001},   {wxT("laquo"), 171},   {wxT("larr"), 8592},
    {wxT("lceil"), 8968},   {wxT("ldquo"), 8220},  {wxT("le"), 8804},     {wxT("lfloor"), 8970}, {wxT("lowast"), 8727},
    {wxT("loz"), 9674},     {wxT("lrm"), 8206},    {wxT("lsaquo"), 8249}, {wxT("lsquo"), 8216},  {wxT("lt"), 60},
    {wxT("macr"), 175},     {wxT("mdash"), 8212},  {wxT("micro"), 181},   {wxT("middot"), 183},  {wxT("minus"), 8722},
    {wxT("mu"), 956},       {wxT("nabla"), 8711},  {wxT("nbsp"), 160},    {wxT("ndash"), 8211},  {wxT("ne"), 8800},
    {wxT("ni"), 8715},      {wxT("not"), 172},     {wxT("notin"), 8713},  {wxT("nsub"), 8836},   {wxT("ntilde"), 241},
    {wxT("nu"), 957},       {wxT("oacute"), 243},  {wxT("ocirc"), 244},   {wxT("oelig"), 339},   {wxT("ograve"), 242},
    {wxT("oline"), 8254},   {wxT("omega"), 969},   {wxT("omicron"), 959}, {wxT("oplus"), 8853},  {wxT("or"), 8744},
    {wxT("ordf"), 170},     {wxT("ordm"), 186},    {wxT("oslash"), 248},  {wxT("otilde"), 245},  {wxT("otimes"), 8855},
    {wxT("ouml"), 246},     {wxT("para"), 182},    {wxT("part"), 8706},   {wxT("permil"), 8240}, {wxT("perp"), 8869},
    {wxT("phi"), 966},      {wxT("pi"), 960},      {wxT("piv"), 982},     {wxT("plusmn"), 177},  {wxT("pound"), 163},
    {wxT("prime"), 8242},   {wxT("prod"), 8719},   {wxT("prop"), 8733},   {wxT("psi"), 968},     {wxT("quot"), 34},
    {wxT("rArr"), 8658},    {wxT("radic"), 8730},  {wxT("rang"), 9002},   {wxT("raquo"), 187},   {wxT("rarr"), 8594},
    {wxT("rceil"), 8969},   {wxT("rdquo"), 8221},  {wxT("real"), 8476},   {wxT("reg"), 174},     {wxT("rfloor"), 8971},
    {wxT("rho"), 961},      {wxT("rlm"), 8207},    {wxT("rsaquo"), 8250}, {wxT("rsquo"), 8217},  {wxT("sbquo"), 8218},
    {wxT("scaron"), 353},   {wxT("sdot"), 8901},   {wxT("sect"), 167},    {wxT("shy"), 173},     {wxT("sigma"), 963},
    {wxT("sigmaf"), 962},   {wxT("sim"), 8764},    {wxT("spades"), 9824}, {wxT("sub"), 8834},    {wxT("sube"), 8838},
    {wxT("sum"), 8721},     {wxT("sup"), 8835},    {wxT("sup1"), 185},    {wxT("sup2"), 178},    {wxT("sup3"), 179},
    {wxT("supe"), 8839},    {wxT("szlig"), 223},   {wxT("tau"), 964},     {wxT("there4"), 8756}, {wxT("theta"), 952},
    {wxT("thetasym"), 977}, {wxT("thinsp"), 8201}, {wxT("thorn"), 254},   {wxT("tilde"), 732},   {wxT("times"), 215},
    {wxT("trade"), 8482},   {wxT("uArr"), 8657},   {wxT("uacute"), 250},  {wxT("uarr"), 8593},   {wxT("ucirc"), 251},
    {wxT("ugrave"), 249},   {wxT("uml"), 168},     {wxT("upsih"), 978},   {wxT("upsilon"), 965}, {wxT("uuml"), 252},
    {wxT("weierp"), 8472},  {wxT("xi"), 958},      {wxT("yacute"), 253},  {wxT("yen"), 165},     {wxT("yuml"), 255},
    {wxT("zeta"), 950},     {wxT("zwj"), 8205},    {wxT("zwnj"), 8204},   {nullptr, 0}};

}

HHCParser::HHCParser(wxFontEncoding enc, wxTreeCtrl* tree, CHMListCtrl* list) : _tree(tree), _list(list), _enc(enc)
{
    _cvPtr = createCSConvPtr(_enc);

    if (_tree)
        _parents[_level] = _tree->GetRootItem();
}

void HHCParser::parse(const char* chunk)
{
    while (*chunk) {
        switch (*chunk) {
        case '\"':
            _inquote = !_inquote;
            break;

        case '<':
            if (!_inquote) {
                _intag = true;
                _tag   = "";
                ++chunk;
                continue;
            }
            // fallthrough

        case '>':
            if (!_inquote) {
                handleTag(_tag);
                _intag = false;
                ++chunk;
                continue;
            }
            // fallthrough
        }

        if (_intag)
            _tag += *chunk;
        ++chunk;
    }
}

void HHCParser::handleTag(const std::string& tag)
{
    constexpr auto TIME_TO_YIELD = 1024;

    if (tag.empty())
        return;

    ++_counter;

    if ((_counter % TIME_TO_YIELD) == 0) {
        wxYield();
        _counter = 0;
    }

    size_t i;
    for (i = 0; i < tag.length(); ++i) {
        if (isspace(tag[i]))
            continue;
        else
            break;
    }

    if (i == tag.length())
        return;

    std::string tagName;

    for (; i != tag.length(); ++i) {
        if (!isspace(tag[i]))
            tagName += tolower(tag[i]);
        else
            break;
    }

    if (_inobject) {
        if (tagName == "/object") {
            _inobject = false;

            if (!_value.empty() && _value[0] != '/')
                _value = "/" + _value;

            auto name  = CURRENT_CHAR_STRING(_name.c_str());
            auto value = CURRENT_CHAR_STRING(_value.c_str());

            if (_htmlChars) {
                name       = replaceHTMLChars(name);
                value      = replaceHTMLChars(value);
                _htmlChars = false;
            }

            name = translateEncoding(name, _enc);

            addToTree(name, value);
            addToList(name, value);

        } else if (tagName == "param") {
            std::string name, value;
            auto        special = getParameters(tag.c_str() + i, name, value);

            if (name == "name" && _name.empty()) {
                _name      = value;
                _htmlChars = _htmlChars || special;
            } else if (name == "local" && _value.empty()) {
                _value     = value;
                _htmlChars = _htmlChars || special;
            }
        }

    } else {
        if (tagName == "ul")
            ++_level;
        else if (tagName == "/ul") {
            if (_level > 0)
                --_level;
        } else if (tagName == "object") {
            _name = _value = "";
            _inobject      = true;
        }
    }
}

bool HHCParser::getParameters(const char* input, std::string& name, std::string& value)
{
    auto lower = false, modify = false;

    name = value = "";

    while (*input) {
        std::string tmpstr;

        while (*input && isspace(*input))
            ++input;

        while (*input && !isspace(*input) && *input != '=') {
            tmpstr += tolower(*input);
            ++input;
        }

        while (*input && isspace(*input))
            ++input;

        if (*input) {
            if (*input != '=')
                return modify;
            else
                ++input;
        }

        if (tmpstr == "name")
            lower = true;
        else if (tmpstr == "value")
            lower = false;
        else {
            // now skip value.
            while (*input && isspace(*input))
                ++input;

            if (*input == '\"') {
                ++input;
                while (*input && *input != '\"')
                    ++input;
                if (*input == '\"')
                    ++input;
            } else {
                while (*input && !isspace(*input))
                    ++input;
            }
            continue;
        }

        while (*input && isspace(*input))
            ++input;

        if (*input == '\"') {
            ++input;
            while (*input && *input != '\"') {
                if (lower)
                    name += tolower(*input++);
                else {
                    if (*input == '&')
                        modify = true;
                    value += *input++;
                }
            }

            if (*input == '\"')
                ++input;
        } else {
            while (*input && !isspace(*input))
                if (lower)
                    name += tolower(*input++);
                else {
                    if (*input == '&')
                        modify = true;
                    value += *input++;
                }
        }
    }

    return modify;
}

void HHCParser::addToTree(const wxString& name, const wxString& value)
{
    if (!_tree)
        return;

    if (!name.IsEmpty()) {
        auto parentIndex = _level ? _level - 1 : 0;

        _parents[_level] = _tree->AppendItem(_parents[parentIndex], name, 2, 2, new URLTreeItem(value));

        if (!_level)
            _parents[0] = _tree->GetRootItem();
        else {
            if (_tree->GetItemImage(_parents[parentIndex]) != 0) {
                _tree->SetItemImage(_parents[parentIndex], 0, wxTreeItemIcon_Normal);
                _tree->SetItemImage(_parents[parentIndex], 0, wxTreeItemIcon_Selected);
                _tree->SetItemImage(_parents[parentIndex], 1, wxTreeItemIcon_Expanded);
            }
        }
    }
}

void HHCParser::addToList(const wxString& name, const wxString& value)
{
    if (!_list)
        return;

    if (!name.IsEmpty() && !value.IsEmpty())
        _list->AddPairItem(name, value);
}

wxString HHCParser::replaceHTMLChars(const wxString& input)
{
    wxString result;

    if (input.IsEmpty())
        return wxEmptyString;

    auto     inSpecial = false;
    wxString special;

    for (size_t i = 0; i < input.Length(); ++i) {
        switch (wxChar(input[i])) {
        case wxT('&'):
            inSpecial = true;
            special   = wxEmptyString;
            continue;
        case wxT(';'):
            if (inSpecial) {
                inSpecial = false;

                auto code = getHTMLCode(special);

                if (code)
                    result.Append(charForCode(code, *_cvPtr, false));

                continue;
            }
        }

        if (inSpecial)
            special.Append(input[i]);
        else if (input[i])
            result.Append(input[i]);
    }

    return result;
}

unsigned HHCParser::getHTMLCode(const wxString& name)
{
    auto substitutions_cnt = sizeof(substitutions) / sizeof(HTMLChar) - 1;
    auto hc                = static_cast<HTMLChar*>(
        bsearch(name.c_str(), substitutions, substitutions_cnt, sizeof(HTMLChar), HTMLCharCompare));

    return hc ? hc->code : 0;
}
