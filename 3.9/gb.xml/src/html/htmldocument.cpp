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


#include "htmlelement.h"
#include "htmldocument.h"
#include <memory.h>
#include <stdlib.h>

Element* GetElement(Node *parent, const char *tagName, const size_t lenTagName);
Attribute* GetAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName);

void UpdateMetaCharset(Document *doc, bool html5);

Document *HtmlDocument_New()
{
    Document *newDoc = XML.XMLDocument_New();
    newDoc->docType = XHTMLDocumentType;
    newDoc->root->parentDocument = newDoc;

    XML.XMLElement_SetTagName(newDoc->root, "html", 4);
    
    Element *head = XML.XMLElement_New("head", 4);
    XML.XMLNode_appendChild(newDoc->root, head);
    
    Element *body = XML.XMLElement_New("body", 4);
    XML.XMLNode_appendChild(newDoc->root, body);
    
    //Meta utf-8
    Element *meta = XML.XMLElement_New("meta", 4);

    XML.XMLElement_AddAttribute(meta, "http-equiv", 10, "Content-Type", 12);
    XML.XMLElement_AddAttribute(meta, "content", 7,"text/html; charset=utf-8", 24);
    XML.XMLNode_appendChild(head, meta);
    
    //Title
    Element *title = XML.XMLElement_New("title", 5);
    XML.XMLNode_appendChild(head, title);

    return newDoc;
}

Document* HtmlDocument_NewFromFile(const char *fileName, const size_t lenFileName)
{
    return XML.XMLDocument_NewFromFile(fileName, lenFileName, XHTMLDocumentType);
}

void HtmlDocument_SetHTML(Document *doc, const bool isHtml)
{
    UpdateMetaCharset(doc, isHtml);
    if(isHtml)
    {
        doc->docType = HTMLDocumentType;
    }
    else
    {
        doc->docType = XHTMLDocumentType;
    }
}

Element* HtmlDocument_GetHead(Document *doc)
{
    return GetElement(doc->root, "head", 4);
}

Element* HtmlDocument_GetBody(Document *doc)
{
    return GetElement(doc->root, "body", 4);
}

Element* HtmlDocument_GetTitle(Document *doc)
{
    return GetElement(HtmlDocument_GetHead(doc), "title", 5);
}

Attribute* HtmlDocument_GetFavicon(Document *doc)
{
    Element *head = HtmlDocument_GetHead(doc);
    Element **elmts; size_t lenElmts;
    elmts = XML.XMLNode_getChildrenByTagName(head, "link", 4, lenElmts, 2);
    Element *elmt;

    Attribute *attr;

    for(unsigned int i = 0; i < lenElmts; i++)
    {
        attr = XML.XMLElement_GetAttribute(elmts[i], "rel", 3, 0);
        if(attr->lenAttrValue == 4)
        {
            if(!memcmp(attr->attrValue, "icon", 4))
            {
                elmt = elmts[i];
                free(elmts);
                return XML.XMLElement_GetAttribute(elmt, "href", 4, 0);
            }
        }
    }

    free(elmts);
    elmt = XML.XMLElement_New("link", 4);
    XML.XMLElement_AddAttribute(elmt, "rel", 3, "icon", 4);
    XML.XMLNode_appendChild(head, elmt);
    return GetAttribute(elmt, "href", 4);
}


Attribute* HtmlDocument_GetBase(Document *doc)
{
    return GetAttribute(GetElement(HtmlDocument_GetHead(doc), "base", 4), "href", 4);
}

Attribute* HtmlDocument_GetLang(Document *doc)
{
    return GetAttribute(doc->root, "lang", 4);
}

void HtmlDocument_AddStyleSheet(Document *doc, const char *src, size_t lenSrc,
                                 const char *media, size_t lenMedia)
{
    Element *elmt = XML.XMLElement_New("link", 4);
    XML.XMLElement_AddAttribute(elmt, "rel", 3, "stylesheet", 10);
    XML.XMLElement_AddAttribute(elmt, "href", 4, src, lenSrc);
    XML.XMLElement_AddAttribute(elmt, "type", 4, "text/css", 8);
    XML.XMLElement_AddAttribute(elmt, "media", 5, media, lenMedia);
    XML.XMLNode_appendChild(HtmlDocument_GetHead(doc),elmt);
}

void HtmlDocument_AddStyleSheetIfIE(Document *doc, const char *src, size_t lenSrc,
                                     const char *cond, size_t lenCond, 
                                     const char *media, size_t lenMedia)
{
//[if +cond+]><link rel="stylesheet" href="+src+" type="text/css" media="+media+" /><![endif]
    size_t lenContent = 74 + lenCond + lenSrc + lenMedia;
    char *content = (char*)malloc(lenContent);
    
    memcpy(content, "[if ", 4);
    content += 4;
    memcpy(content, cond, lenCond);
    content += lenCond;
    memcpy(content, "]><link rel=\"stylesheet\" href=\"", 32);
    content += 32;
    memcpy(content, src, lenSrc);
    content += lenSrc;
    memcpy(content, "\" type=\"text/css\" media=\"", 25);
    content += 25;
    memcpy(content, media, lenMedia);
    content += lenMedia;
    memcpy(content, "\" /><![endif]", 13);
    content -= lenContent - 13;

    CommentNode *comment = XML.XMLComment_New(content, lenContent);
    XML.XMLNode_appendChild(HtmlDocument_GetHead(doc),comment);
}

void HtmlDocument_AddStyleSheetIfNotIE(Document *doc, const char *src, size_t lenSrc,
                                        const char *media, size_t lenMedia)
{
    Element *head = HtmlDocument_GetHead(doc);
    CommentNode *comment =  XML.XMLComment_New("[if !IE]><", 10);
    XML.XMLNode_appendChild(head,comment);
    HtmlDocument_AddStyleSheet(doc, src, lenSrc, media, lenMedia);
    comment =  XML.XMLComment_New("><![endif]", 10);
    XML.XMLNode_appendChild(head,comment);
}

void HtmlDocument_AddScript(Document *doc, const char *src, size_t lenSrc)
{
    Element *elmt = XML.XMLElement_New("script", 6);
    XML.XMLElement_AddAttribute(elmt, "src", 3, src, lenSrc);
    XML.XMLElement_AddAttribute(elmt, "type", 4, "text/javascript", 15);
    XML.XMLNode_appendChild(HtmlDocument_GetHead(doc),elmt);
}

void HtmlDocument_AddScriptIfIE(Document *doc, const char *src, size_t lenSrc,
                                 const char *cond, size_t lenCond)
{
    
//[if +cond+]><script src="+src+" type="text/javascript"></script><![endif]
    size_t lenContent = 44 + lenCond + lenSrc;
    char *content = (char*)malloc(lenContent);
    
    memcpy(content, "[if ", 4);
    content += 4;
    memcpy(content, cond, lenCond);
    content += lenCond;
    memcpy(content, "]><script src=\"", 14);
    content += 14;
    memcpy(content, src, lenSrc);
    content += lenSrc;
    memcpy(content, "\" type=\"text/javascript\"></script><![endif]", 26);
    content -= lenContent - 26;

    CommentNode *comment = XML.XMLComment_New(content, lenContent);
    XML.XMLNode_appendChild(HtmlDocument_GetHead(doc),comment);
}

void HtmlDocument_AddScriptIfNotIE(Document *doc, const char *src, size_t lenSrc)
{
    Element *head = HtmlDocument_GetHead(doc);
    
    CommentNode *comment = XML.XMLComment_New("[if !IE]><", 10);
    XML.XMLNode_appendChild(head,comment);
    
    HtmlDocument_AddScript(doc, src, lenSrc);
    
    comment = XML.XMLComment_New("><![endif]", 10);
    XML.XMLNode_appendChild(head,comment);
}

Element* HtmlDocument_GetElementById(Document *doc, const char *id, const size_t lenId, int depth)
{
    return XML.XMLNode_getFirstChildByAttributeValue(doc, "id", 2, id, lenId, 0, depth);
}

void HtmlDocument_GetElementsByClassName(Document *doc, const char *className, const size_t lenClassName, GB_ARRAY *array, int depth)
{
    XML.XMLNode_getGBChildrenByAttributeValue(doc, "class", 5, className, lenClassName, array, 0, depth);
}

//Some utils

Element* GetElement(Node *parent, const char *tagName, const size_t lenTagName)
{
    Element *elmt = XML.XMLNode_getFirstChildByTagName(parent, tagName, lenTagName, 2);

    if(!elmt)
    {
        elmt = XML.XMLElement_New(tagName, lenTagName);
        XML.XMLNode_appendChild(parent, elmt);
    }

    return elmt;
}

Attribute* GetAttribute(Element *elmt, const char *nattrName, const size_t nlenAttrName)
{
    Attribute *attr = XML.XMLElement_GetAttribute(elmt, nattrName, nlenAttrName, 0);
    if(!attr) attr = XML.XMLElement_AddAttribute(elmt, nattrName, nlenAttrName, "", 0);
    return attr;
}

void UpdateMetaCharset(Document *doc, bool html5)
{
    if(doc->docType == (html5 ? HTMLDocumentType : XHTMLDocumentType)) return;

    //Looking for the meta charset element
    size_t lenMetas;
    Element **metas = XML.XMLNode_getChildrenByTagName(HtmlDocument_GetHead(doc), "meta", 4, lenMetas, 2);
    Element *meta = 0;
    Attribute *attr;

    for(size_t i = 0; i < lenMetas; i++)
    {
        if(doc->docType == XHTMLDocumentType)
        {
            attr = XML.XMLElement_GetAttribute(metas[i], "http-equiv", 11, 0);
            if(!attr) continue;
            if(!XML.GB_MatchString(attr->attrValue, attr->lenAttrValue, "Content-Type", 12, 0)) continue;
            XML.XMLElement_RemoveAttribute(metas[i], attr);
            attr = XML.XMLElement_GetAttribute(metas[i], "content", 7, 0);
            if(!attr) continue;
            if(!XML.GB_MatchString(attr->attrValue, attr->lenAttrValue, "text/html; charset=utf-8", 25, 0)) continue;
            XML.XMLElement_RemoveAttribute(metas[i], attr);
            meta = metas[i];
            break;
        }
        else
        {
            attr = XML.XMLElement_GetAttribute(metas[i], "charset", 7, 0);
            if(!attr) continue;
            if(!XML.GB_MatchString(attr->attrValue, attr->lenAttrValue, "utf-8", 5, 0)) continue;
            XML.XMLElement_RemoveAttribute(metas[i], attr);

            meta = metas[i];
            break;
        }
    }
    free(metas);

    if(!meta)
    {
        meta = XML.XMLElement_New("meta", 4);
        XML.XMLNode_appendChild(HtmlDocument_GetHead(doc), meta);
    }

    if(html5)
    {
        XML.XMLElement_AddAttribute(meta, "charset", 7, "utf-8", 5);
    }
    else
    {
        XML.XMLElement_AddAttribute(meta, "http-equiv", 11, "Content-Type", 12);
        XML.XMLElement_AddAttribute(meta, "content", 7, "text/html; charset=utf-8", 25);
    }

}

