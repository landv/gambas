/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include "main.h"

#define CHAR_ERROR 0xFFFD // �

#ifdef OS_MACOSX
#include <string.h>
void *memrchr(const char *s, int c, size_t n);
#endif

wchar_t nextUTF8Char(const char *&data, size_t len);
const void* memchrs(const char *source, size_t lensource, const char *comp, size_t lencomp);
const void* memrchrs(const char *source, size_t lensource, const char *comp, size_t lencomp);


bool isNameStartChar(const wchar_t car);
bool isNameChar(const wchar_t car);
bool isWhiteSpace(const wchar_t s);
bool isWhiteSpace(const char s);

void Trim(const char* &str, size_t &len);
void insertString(char *&src, size_t &lenSrc, const char *insert, size_t lenInsert, char *&posInsert);

bool GB_MatchString(const char *str, size_t lenStr, const char *pattern, size_t lenPattern, int mode = GB_STRCOMP_BINARY);

XMLParseException XMLParseException_New(const char *nerror, size_t posFailed);
XMLParseException XMLParseException_New(const char *nerror, const char *data, const size_t lenData, const char *posFailed) throw();
void XMLParseException_Cleanup(XMLParseException *ex);
void ThrowXMLParseException(const char* nerror, const char *text, const size_t lenText, const char *posFailed);


#endif // UTILS_H

#if !defined(UTILS_GBINTERFACE) && defined(GBINTERFACE_H)
#define UTILS_GBINTERFACE

void XML_Format(GB_VALUE *value, char* &dst, size_t &lenDst);

#endif
