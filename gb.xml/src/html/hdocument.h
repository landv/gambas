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

#ifndef HDOCUMENT_H
#define HDOCUMENT_H

#include "../document.h"

class Element;

class HtmlDocument : public Document
{
public:
    
    HtmlDocument();
    HtmlDocument(const char *fileName, const size_t lenFileName);
    

    //virtual void toString(char **output, size_t *len);
    //virtual void toGBString(char **output, size_t *len);
    virtual void addStringLen(size_t &len, int indent = -1);
    virtual void addString(char *&data, int indent = -1);


    Element* getBody();
    Element* getHead();

    Element* getElementById(char *id, size_t lenId, int depth);
    void getElementsByClassName(char* className, size_t lenClassName, GB_ARRAY *array, int depth = -1);

    virtual void setContent(char *content, size_t len) throw(XMLParseException);

    void getGBTitle(char *&title, size_t &len);
    void setTitle(char *title, size_t len);

    void getGBFavicon(char *&favicon, size_t &len);
    void setFavicon(char *title, size_t len);

    void getGBBase(char *&base, size_t &len);
    void setBase(char *content, size_t len);

    void getGBLang(char *&lang, size_t &len);
    void setLang(char *content, size_t len);

    void AddStyleSheet(const char *src, size_t lenSrc,
                       const char *media, size_t lenMedia);
    void AddStyleSheetIfIE(const char *src, size_t lenSrc,
                           const char *cond, size_t lenCond,
                           const char *media, size_t lenMedia);
    void AddStyleSheetIfNotIE(const char *src, size_t lenSrc,
                              const char *media, size_t lenMedia);
    void AddScript(const char *src, size_t lenSrc);
    void AddScriptIfIE(const char *src, size_t lenSrc,
                       const char *cond, size_t lenCond);
    void AddScriptIfNotIE(const char *src, size_t lenSrc);

    Element* getTitleElement();
    Element* getFaviconElement();
    Element* getBaseElement();
    bool html5;

};

#endif
