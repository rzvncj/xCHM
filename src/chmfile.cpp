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
#include <endianmacros.h>
#include <assert.h>


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

	_chmFile = chm_open(archiveName.c_str());
	
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
	LONGINT64 ret = BUF_SIZE2 - 1, curr = 0;

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
		ret = RetrieveObject(&ui, (unsigned char *)buffer, curr,
				     BUF_SIZE2 - 1);
		buffer[ret] = '\0';
		src.Append(buffer);
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


bool CHMFile::ResolveObject(const wxString& fileName, chmUnitInfo *ui)
{
	return _chmFile != NULL && chm_resolve_object(_chmFile, 
						      fileName.c_str(), 
						      ui)
		== CHM_RESOLVE_SUCCESS;
}


size_t CHMFile::RetrieveObject(chmUnitInfo *ui, unsigned char *buffer,
			       off_t fileOffset, size_t bufferSize)
{
	return chm_retrieve_object(_chmFile, ui, buffer, fileOffset,
				   bufferSize);
}


bool CHMFile::GetArchiveInfo()
{
#define BUF_SIZE 2048
	unsigned char buffer[BUF_SIZE];
	chmUnitInfo ui;
	
	int index = 0;
	u_int16_t *cursor = NULL;

	// Do we have the #SYSTEM file in the archive?
	if(chm_resolve_object(_chmFile, "/#SYSTEM", &ui) !=
	   CHM_RESOLVE_SUCCESS)
		return false;

	// Can we pull BUFF_SIZE bytes of the #SYSTEM file?
	if(chm_retrieve_object(_chmFile, &ui,
			       buffer, 4, BUF_SIZE) == 0)
		return false;

	// 9 is a good magic number :)
	for(int i=0; i<9; ++i) {

		cursor = (u_int16_t *)(buffer + index);
		FIXENDIAN16(*cursor);

		switch(*cursor) {
		case 0:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_topicsFile = wxString(wxT("/")) 
				+ (buffer + index + 2);
			index += *cursor + 2;
			break;
		case 1:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_indexFile = wxString(wxT("/"))
				+ (buffer + index + 2);
			index += *cursor + 2;
			break;
		case 2:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_home = wxString(wxT("/"))
				+ (buffer + index + 2);
			index += *cursor + 2;
			break;
		case 3:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);			
			_title = wxString(buffer + index + 2);
			index += *cursor + 2;
			break;
		case 6:
			index += 2;
			cursor = (u_int16_t *)(buffer + index);
			FIXENDIAN16(*cursor);

			if(_topicsFile.IsEmpty()) {
				wxString topicAttempt = wxT("/"), tmp;
				topicAttempt += wxString(buffer + index + 2);
				tmp = topicAttempt + wxT(".hhc");
				
				if(chm_resolve_object(_chmFile,
						      tmp.c_str(), &ui)
				   == CHM_RESOLVE_SUCCESS)
					_topicsFile = tmp;

				tmp = topicAttempt + wxT(".hhk");
				
				if(chm_resolve_object(_chmFile,
						      tmp.c_str(), &ui)
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

		if(chm_resolve_object(_chmFile, "/#STRINGS", &ui) !=
		   CHM_RESOLVE_SUCCESS)
			// no /#STRINGS, but we did get some info.
			return true;

		// Can we pull BUFF_SIZE bytes of the #SYSTEM file?
		if(chm_retrieve_object(_chmFile, &ui,
				       buffer, 1, BUF_SIZE) == 0)
			// this should never happen, the specs say
			// the least the file can be is 4096 bytes.
			return true;

		// There's that 9 again :).
		for(int i=0; i<9; ++i) {
			chunk = buffer + index;
			
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

