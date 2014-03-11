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

#include "CNode.h"
#include "CElement.h"
#include "CTextNode.h"
#include "CDocument.h"
#include "CExplorer.h"
#include "CReader.h"

#include "node.h"
#include "document.h"
#include "element.h"
#include "textnode.h"
#include "serializer.h"
#include "parser.h"

#include "../gambas.h"
#include "html/gb.xml.html.h"

GB_INTERFACE GB EXPORT;
XML_HTML_INTERFACE HTML EXPORT;

extern "C"
{

    GB_DESC *GB_CLASSES[] EXPORT =
    {
       CNodeDesc, CElementDesc, CTextNodeDesc, CCommentNodeDesc, CCDATANodeDesc, CElementAttributesDesc, CElementAttributeNodeDesc,
       CDocumentDesc, 
       CExplorerDesc, 
       CReaderDesc, CReaderNodeAttributesDesc, CReaderNodeDesc, CReaderNodeTypeDesc, CReaderReadFlagsDesc, 0
    };

    void *GB_XML_1[] EXPORT =
    {
        (void *)XML_INTERFACE_VERSION,
        (void *)serializeXMLNode,
        (void *)GBserializeXMLNode,
        (void *)parseXML,
        (void *)GBGetXMLTextContent,

        (void *)XMLText_escapeContent,
        (void *)XMLText_unEscapeContent,
        (void *)XMLText_escapeAttributeContent,

        (void *)XMLNode_GetGBObject,
        (void *)XMLNode_getFirstChildByAttributeValue,
        (void *)XMLNode_getGBChildrenByAttributeValue,
        (void *)XMLNode_firstChildElement,
        (void *)XMLNode_lastChildElement,
        (void *)XMLNode_previousElement,
        (void *)XMLNode_appendChild,
        (void *)XMLNode_getChildrenByTagName,
        (void *)XMLNode_getFirstChildByTagName,
        (void *)XMLNode_setTextContent,

        (void *)static_cast<TextNode* (*)(const char*, size_t)>(XMLTextNode_New),
        (void *)XMLTextNode_setEscapedTextContent,
        (void *)XMLTextNode_checkEscapedContent,

        (void *) XMLDocument_New,
        (void *) XMLDocument_NewFromFile,
        (void *)XMLDocument_SetContent,

        (void *) static_cast<Element* (*)(const char*, size_t)>(XMLElement_New),
        (void *)XMLElement_SetTagName,
        (void *)XMLElement_AttributeContains,
        (void *) static_cast<Attribute* (*)(Element*, const char*, size_t, const char*, size_t)>(XMLElement_AddAttribute),
        (void *)XMLElement_GetAttribute,
        (void *)XMLElement_SetAttribute,
        (void *) static_cast<void (*)(Element*, Attribute*)>(XMLElement_RemoveAttribute),
        (void *) static_cast<CommentNode* (*)(const char*, size_t)>(XMLComment_New),

        (void *) static_cast<CDATANode* (*)(const char*, size_t)>(XMLCDATA_New),

        (void *)XML_ReturnNode,

        (void *)Trim,
        (void *)isNameStartChar,
        (void *)isNameChar,
        (void *)memchrs,
        (void *)GB_MatchString,
        (void *)nextUTF8Char,
        (void *) static_cast<bool (*)(const wchar_t s)>(isWhiteSpace),

        (void *)ThrowXMLParseException,

    #if defined(OS_MACOSX) || defined(__APPLE__)
        (void*)memrchr,
    #endif

        NULL
    };
    
    int EXPORT GB_INIT(void)
    {
        memset(&HTML, 0, sizeof(XML_HTML_INTERFACE));
      return -1;
    }
    
    void EXPORT GB_EXIT()
    {
    
    }
}

bool CheckHtmlInterface()
{
    if((int)HTML.version == XML_HTML_INTERFACE_VERSION)
    {
        return true;
    }
    if(GB.ExistClass("HtmlDocument"))
    {
        GB.GetInterface("gb.xml.html", XML_HTML_INTERFACE_VERSION, &HTML);
        return true;
    }
    return false;
}

