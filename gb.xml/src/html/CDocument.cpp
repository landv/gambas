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

#include "CDocument.h"
#include "hdocument.h"
#include "helement.h"
#include "../gbi.h"

#define STRINGOPT(_str, _repl, _lenrepl) MISSING(_str) ? _repl : STRING(_str),\
    MISSING(_str) ? _lenrepl : LENGTH(_str)

/*========== Document */

#undef THIS
#define THIS ((HtmlDocument*)(static_cast<CDocument*>(_object)->doc))

BEGIN_METHOD(CDocument_new, GB_STRING path)

//(static_cast<CDocument*>(_object)->doc) = new HtmlDocument;

if(!MISSING(path))
{
    (static_cast<CDocument*>(_object)->doc) = new HtmlDocument(STRING(path), LENGTH(path));
}
else
{
    (static_cast<CDocument*>(_object)->doc) = new HtmlDocument;
}


END_METHOD

BEGIN_METHOD_VOID(CDocument_free)

//GB.Unref(POINTER(&(THIS->root)));

END_METHOD

BEGIN_METHOD(CDocument_createElement, GB_STRING tagName)

Element *elmt = new Element(STRING(tagName), LENGTH(tagName));
Node::NoInstanciate = true;
GBI::Return(elmt);
Node::NoInstanciate = false;

END_METHOD

BEGIN_PROPERTY(CDocument_Html5)

if(READ_PROPERTY)
{
    GB.ReturnBoolean(THIS->html5);
}
else
{
    THIS->html5 = VPROP(GB_BOOLEAN);
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_Title)

if(READ_PROPERTY)
{
    char *title; size_t lenTitle;
    THIS->getGBTitle(title, lenTitle);
    GB.ReturnString(title);
}
else
{
if(PLENGTH() <= 0) return;
THIS->setTitle(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_favicon)

if(READ_PROPERTY)
{
    char *favicon; size_t lenFavicon;
    THIS->getGBFavicon(favicon, lenFavicon);
    GB.ReturnString(favicon);
}
else
{
if(PLENGTH() <= 0) return;
THIS->setFavicon(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_lang)

if(READ_PROPERTY)
{
    char *lang; size_t lenLang;
    THIS->getGBLang(lang, lenLang);
    GB.ReturnString(lang);
}
else
{
    if(PLENGTH() <= 0) return;
    THIS->setLang(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_root)

Node::NoInstanciate = true;
GBI::Return(THIS->root);
Node::NoInstanciate = false;

END_PROPERTY

BEGIN_PROPERTY(CDocument_head)

Node::NoInstanciate = true;
GBI::Return(THIS->getHead());
Node::NoInstanciate = false;

END_PROPERTY

BEGIN_PROPERTY(CDocument_body)

Node::NoInstanciate = true;
GBI::Return(THIS->getBody());
Node::NoInstanciate = false;

END_PROPERTY

BEGIN_METHOD(CDocument_getElementById, GB_STRING id; GB_INTEGER depth)

Node::NoInstanciate = true;
GBI::Return(THIS->getElementById(STRING(id), LENGTH(id), VARGOPT(depth, -1)));
Node::NoInstanciate = false;

END_METHOD

BEGIN_METHOD(CDocument_getElementsByClassName, GB_STRING className; GB_INTEGER depth)

Node::NoInstanciate = true;
if(LENGTH(className) <= 0) return;
GB_ARRAY array;
THIS->getElementsByClassName(STRING(className), LENGTH(className), &array, VARGOPT(depth, -1));
GB.ReturnObject(array);
Node::NoInstanciate = false;

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_add, GB_STRING path; GB_STRING media)

THIS->AddStyleSheet(STRING(path), LENGTH(path), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_addIfNotIE, GB_STRING path; GB_STRING media)

THIS->AddStyleSheetIfNotIE(STRING(path), LENGTH(path), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_addIfIE, GB_STRING path; GB_STRING cond; GB_STRING media)

THIS->AddStyleSheetIfIE(STRING(path), LENGTH(path), 
                        STRINGOPT(cond, "IE", 2), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentScripts_add, GB_STRING path)

THIS->AddScript(STRING(path), LENGTH(path));

END_METHOD

BEGIN_METHOD(CDocumentScripts_addIfNotIE, GB_STRING path)

THIS->AddScriptIfNotIE(STRING(path), LENGTH(path));

END_METHOD

BEGIN_METHOD(CDocumentScripts_addIfIE, GB_STRING path; GB_STRING cond)

THIS->AddScriptIfIE(STRING(path), LENGTH(path), STRINGOPT(cond, "IE", 2));

END_METHOD

/*BEGIN_METHOD(CDocument_forceSetContent, GB_STRING data)

try
{
    THIS->setContent(STRING(data), true);
}
catch(HTMLParseException &e)
{
    GB.Error(e.what());
}

END_METHOD*/

BEGIN_PROPERTY(CDocument_base)

if(READ_PROPERTY)
{
    char *base; size_t lenBase;
    THIS->getGBBase(base, lenBase);
    GB.ReturnString(base);
}
else
{
THIS->setBase(PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_METHOD(CDocument_fromString, GB_STRING content)

try
{
    THIS->setContent(STRING(content), LENGTH(content));
}
catch(XMLParseException &e)
{
    GB.Error(e.what());
}

END_METHOD


GB_DESC CDocumentStyleSheetsDesc[] =
{
    GB_DECLARE(".HtmlDocumentStyleSheets", 0), GB_VIRTUAL_CLASS(),

    GB_METHOD("Add", "", CDocumentStyleSheets_add, "(Source)s[(Media)s]"),
    GB_METHOD("AddIfIE", "", CDocumentStyleSheets_addIfIE, "(Source)s[(Condition)s(Media)s]"),
    GB_METHOD("AddIfNotIE", "", CDocumentStyleSheets_addIfNotIE, "(Source)s[(Media)s]"),

    GB_END_DECLARE
};

GB_DESC CDocumentScriptsDesc[] =
{
    GB_DECLARE(".HtmlDocumentScripts", 0), GB_VIRTUAL_CLASS(),

    GB_METHOD("Add", "", CDocumentScripts_add, "(Source)s"),
    GB_METHOD("AddIfIE", "", CDocumentScripts_addIfIE, "(Source)s[(Condition)s]"),
    GB_METHOD("AddIfNotIE", "", CDocumentScripts_addIfNotIE, "(Source)s"),

    GB_END_DECLARE
};

GB_DESC CDocumentDesc[] =
{
    GB_DECLARE("HtmlDocument", sizeof(CDocument)), GB_INHERITS("XmlDocument"),

    GB_METHOD("_new", "", CDocument_new, "[(Path)s]"),
    GB_METHOD("_free", "", CDocument_free, ""),
    
    
    GB_METHOD("CreateElement", "XmlElement", CDocument_createElement, "(TagName)s"),
    GB_PROPERTY("Html5", "b", CDocument_Html5),
    //GB_METHOD("ForceSetContent", "", CDocument_forceSetContent, "(Data)s"),
    GB_PROPERTY("Title", "s", CDocument_Title),
    GB_PROPERTY("Favicon", "s", CDocument_favicon),
    GB_PROPERTY("Lang", "s", CDocument_lang),
    GB_PROPERTY("Base", "s", CDocument_base),
    GB_PROPERTY_READ("Head", "XmlElement", CDocument_head),
    GB_PROPERTY_READ("Body", "XmlElement", CDocument_body),

    GB_METHOD("FromString", "", CDocument_fromString, "(Data)s"),
    GB_METHOD("HtmlFromString", "", CDocument_fromString, "(Data)s"),

    GB_PROPERTY_SELF("StyleSheets", ".HtmlDocumentStyleSheets"),
    GB_PROPERTY_SELF("Scripts", ".HtmlDocumentScripts"),

    GB_METHOD("GetElementById", "XmlElement", CDocument_getElementById, "(Id)s[(Depth)i]"),
    GB_METHOD("GetElementsByClassName", "XmlElement[]", CDocument_getElementsByClassName, "(ClassName)s[(Depth)i]"),


    GB_END_DECLARE
};
