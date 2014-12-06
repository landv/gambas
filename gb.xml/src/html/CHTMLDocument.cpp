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

#include "CHTMLDocument.h"
#include "htmldocument.h"
#include "htmlelement.h"

/*========== Document */

#define THIS ((Document*)(static_cast<CDocument*>(_object)->node))

BEGIN_METHOD_VOID(CDocument_new)

END_METHOD

BEGIN_METHOD_VOID(CDocument_free)



END_METHOD

BEGIN_PROPERTY(CDocument_Html5)

if(READ_PROPERTY)
{
    GB.ReturnBoolean(THIS->docType == HTMLDocumentType);
}
else
{
    HtmlDocument_SetHTML(THIS, VPROP(GB_BOOLEAN));
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_Title)

if(READ_PROPERTY)
{
    char *title; size_t lenTitle;
    XML.GBGetXMLTextContent(HtmlDocument_GetTitle(THIS), title, lenTitle);
    GB.ReturnString(title);
}
else
{
    if(PLENGTH() <= 0) return;
    XML.XMLNode_setTextContent(HtmlDocument_GetTitle(THIS), PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_favicon)

if(READ_PROPERTY)
{
    char *favicon; size_t lenFavicon;
    XML.GBGetXMLTextContent(HtmlDocument_GetFavicon(THIS), favicon, lenFavicon);
    GB.ReturnString(favicon);
}
else
{
    if(PLENGTH() <= 0) return;
    XML.XMLNode_setTextContent(HtmlDocument_GetFavicon(THIS), PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_lang)

if(READ_PROPERTY)
{
    char *lang; size_t lenLang;
    XML.GBGetXMLTextContent(HtmlDocument_GetLang(THIS), lang, lenLang);
    GB.ReturnString(lang);
}
else
{
    if(PLENGTH() <= 0) return;
    XML.XMLNode_setTextContent(HtmlDocument_GetLang(THIS), PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_base)

if(READ_PROPERTY)
{
    char *base; size_t lenBase;
    XML.GBGetXMLTextContent(HtmlDocument_GetBase(THIS), base, lenBase);
    GB.ReturnString(base);
}
else
{
    if(PLENGTH() <= 0) return;
    XML.XMLNode_setTextContent(HtmlDocument_GetBase(THIS), PSTRING(), PLENGTH());
}

END_PROPERTY

BEGIN_PROPERTY(CDocument_root)

XML.ReturnNode(THIS->root);

END_PROPERTY

BEGIN_PROPERTY(CDocument_head)

XML.ReturnNode(HtmlDocument_GetHead(THIS));

END_PROPERTY

BEGIN_PROPERTY(CDocument_body)

XML.ReturnNode(HtmlDocument_GetBody(THIS));

END_PROPERTY

BEGIN_METHOD(CDocument_getElementById, GB_STRING id; GB_INTEGER depth)

XML.ReturnNode(HtmlDocument_GetElementById(THIS, STRING(id), LENGTH(id), VARGOPT(depth, -1)));

END_METHOD

BEGIN_METHOD(CDocument_getElementsByClassName, GB_STRING className; GB_INTEGER depth)

if(LENGTH(className) <= 0) return;
GB_ARRAY array;
HtmlDocument_GetElementsByClassName(THIS, STRING(className), LENGTH(className), &array, VARGOPT(depth, -1));
GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_add, GB_STRING path; GB_STRING media)

HtmlDocument_AddStyleSheet(THIS, STRING(path), LENGTH(path), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_addIfNotIE, GB_STRING path; GB_STRING media)

HtmlDocument_AddStyleSheetIfNotIE(THIS, STRING(path), LENGTH(path), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentStyleSheets_addIfIE, GB_STRING path; GB_STRING cond; GB_STRING media)

HtmlDocument_AddStyleSheetIfIE(THIS, STRING(path), LENGTH(path),
                        STRINGOPT(cond, "IE", 2), STRINGOPT(media, "screen", 6));

END_METHOD

BEGIN_METHOD(CDocumentScripts_add, GB_STRING path)

HtmlDocument_AddScript(THIS, STRING(path), LENGTH(path));

END_METHOD

BEGIN_METHOD(CDocumentScripts_addIfNotIE, GB_STRING path)

HtmlDocument_AddScriptIfNotIE(THIS, STRING(path), LENGTH(path));

END_METHOD

BEGIN_METHOD(CDocumentScripts_addIfIE, GB_STRING path; GB_STRING cond)

HtmlDocument_AddScriptIfIE(THIS, STRING(path), LENGTH(path), STRINGOPT(cond, "IE", 2));

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

    GB_METHOD("_new", "", CDocument_new, ""),
    GB_METHOD("_free", "", CDocument_free, ""),
    
    GB_PROPERTY("Html5", "b", CDocument_Html5),

    GB_PROPERTY("Title", "s", CDocument_Title),
    GB_PROPERTY("Favicon", "s", CDocument_favicon),
    GB_PROPERTY("Lang", "s", CDocument_lang),
    GB_PROPERTY("Base", "s", CDocument_base),
    GB_PROPERTY_READ("Head", "XmlElement", CDocument_head),
    GB_PROPERTY_READ("Body", "XmlElement", CDocument_body),

    //GB_METHOD("FromString", "", CDocument_fromString, "(Data)s"),
    //GB_METHOD("HtmlFromString", "", CDocument_fromString, "(Data)s"),

    GB_PROPERTY_SELF("StyleSheets", ".HtmlDocumentStyleSheets"),
    GB_PROPERTY_SELF("Scripts", ".HtmlDocumentScripts"),

    GB_METHOD("GetElementById", "XmlElement", CDocument_getElementById, "(Id)s[(Depth)i]"),
    GB_METHOD("GetElementsByClassName", "XmlElement[]", CDocument_getElementsByClassName, "(ClassName)s[(Depth)i]"),


    GB_END_DECLARE
};
