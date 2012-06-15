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

typedef unsigned int uint;

#define DELETE(_ob) if(_ob) {delete _ob; _ob = 0;}
#define UNREF(_ob) if(_ob) GB.Unref(POINTER(&(_ob)))

#define SCHAR_N 0xA // \n
#define SCHAR_R 0xD // \r

#define CHAR_STARTTAG 0x3C // <
#define CHAR_ENDTAG 0x3E // >

#define CHAR_SINGLEQUOTE 0x27 // '
#define CHAR_DOUBLEQUOTE 0x22 // "

#define CHAR_SLASH 0x2F
#define CHAR_BACKSLASH 0x5C
#define CHAR_EQUAL 0x3D // =
#define CHAR_AND 0x26 // &
#define CHAR_PI 0x3F // ?
#define CHAR_EXCL 0x21 // !
#define CHAR_DASH 0x2D // -

#define CHAR_SPACE 0x20 

#define CHAR_a 0x61 // a
#define CHAR_z 0x7A // z

#define CHAR_ERROR 0xFFFD // �

wchar_t nextUTF8Char(char *&data, size_t len);
const void* memchrs(const char *source, size_t lensource, const char *comp, size_t lencomp);
const void* memrchrs(const void *source, size_t lensource, const void *comp, size_t lencomp);


bool isNameStartChar(const wchar_t s);
bool isNameChar(const wchar_t s);
bool isWhiteSpace(const wchar_t s);
bool isWhiteSpace(const char s);

void Trim(char *&str, size_t &len);
void insertString(char *&src, size_t &lenSrc, const char *insert, size_t lenInsert, char *&posInsert);

#include <exception>

class XMLParseException : public exception
{
public:
    XMLParseException(const char* nerror, const char *text, const size_t lenText, const char *posFailed) throw();
    virtual ~XMLParseException() throw();
    
    virtual const char* what() const throw();
    
private:
    void AnalyzeText(const char *text, const size_t lenText, const char *posFailed) throw();
    
    char *near;
    char *error;
    size_t lenError;
    size_t lenNear;
    size_t line;
    size_t column;
    
    char *errorWhat;
};


#endif // UTILS_H
