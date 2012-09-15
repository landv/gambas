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


#include "helement.h"
#include "hdocument.h"
#include "../textnode.h"
#include "../utils.h"

HtmlDocument::HtmlDocument() : Document(), html5(false)
{
    root->setTagName("html", 4);
    
    /*Element *head = new Element;
    head->setTagName("head", 4);
    root->appendChild(head);
    
    Element *body = new Element;
    body->setTagName("body", 4);
    root->appendChild(body);
    
    //Meta utf-8
    Element *meta = new Element;
    meta->setTagName("meta", 4);
    meta->setAttribute("charset", 7, "utf-8", 5);
    head->appendChild(meta);
    
    meta = new Element;
    meta->setTagName("meta", 4);
    meta->setAttribute("http-equiv", 10, "Content-Type", 12);
    meta->setAttribute("content", 7,"text/html; charset=utf-8", 25);
    head->appendChild(meta);
    
    //Title
    Element *title = new Element;
    title->setTagName("title", 5);
    head->appendChild(title);*/
}

HtmlDocument::HtmlDocument(const char *fileName, const size_t lenFileName) : Document(fileName, lenFileName)
{
    
}

Element* HtmlDocument::getTitleElement()
{
    Element *head = getHead();
    Element **elmts; size_t lenElmts;
    elmts = head->getChildrenByTagName("title", 5, lenElmts);
    Element *elmt;
    if(lenElmts <= 0)
    {
        elmt = new Element;
        elmt->setTagName("title", 5);
        head->appendChild(elmt);
    }
    else
    {
        elmt = *elmts;
        free(elmts);
    }
    
    return elmt;
}

Element* HtmlDocument::getBaseElement()
{
    Element *head = getHead();
    Element **elmts; size_t lenElmts;
    elmts = head->getChildrenByTagName("base", 4, lenElmts);
    Element *elmt;
    if(lenElmts <= 0)
    {
        elmt = new Element;
        elmt->setTagName("base", 4);
        head->appendChild(elmt);
    }
    else
    {
        elmt = *elmts;
        free(elmts);
    }
    
    return elmt;
}

Element* HtmlDocument::getFaviconElement()
{
    Element *head = getHead();
    Element **elmts; size_t lenElmts;
    elmts = head->getChildrenByTagName("link", 4, lenElmts);
    Element *elmt;
    
    Attribute *attr;

    for(unsigned int i = 0; i < lenElmts; i++)
    {        
        attr = elmts[i]->getAttribute("rel", 3);
        if(attr->lenAttrValue == 4)
        {
            if(!memcmp(attr->attrValue, "icon", 4))
            {
                elmt = elmts[i];
                free(elmts);
                return elmt;
            }
        }
    }

    free(elmts);
    elmt = new Element("link", 4);
    elmt->setAttribute("rel", 3, "icon", 4);
    head->appendChild(elmt);
    return elmt;

}

void HtmlDocument::getGBBase(char *&base, size_t &len)
{
    Attribute *attr = getBaseElement()->getAttribute("href", 4);
    if(attr)
    {
        base = attr->attrValue;
        len = attr->lenAttrValue;
    }
    else
    {
        base = 0;
        len = 0;
    }
}

void HtmlDocument::setBase(char *content, size_t len)
{
    getBaseElement()->setAttribute("href", 4, content, len);
}

void HtmlDocument::getGBFavicon(char *&base, size_t &len)
{
    Attribute *attr = getFaviconElement()->getAttribute("href", 4);
    if(attr)
    {
        base = attr->attrValue;
        len = attr->lenAttrValue;
    }
    else
    {
        base = 0;
        len = 0;
    }
}

void HtmlDocument::setFavicon(char *content, size_t len)
{
    getFaviconElement()->setAttribute("href", 4, content, len);
}

void HtmlDocument::AddStyleSheet(const char *src, size_t lenSrc, 
                                 const char *media, size_t lenMedia)
{
    Element *elmt = new Element("link", 4);
    elmt->addAttribute("rel", 3, "stylesheet", 10);
    elmt->addAttribute("href", 4, src, lenSrc);
    elmt->addAttribute("type", 4, "text/css", 8);
    elmt->setAttribute("media", 5, media, lenMedia);
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddStyleSheetIfIE(const char *src, size_t lenSrc, 
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

    CommentNode *comment = new CommentNode(content, lenContent);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddStyleSheetIfNotIE(const char *src, size_t lenSrc, 
                                        const char *media, size_t lenMedia)
{
    Element *head = getHead();
    CommentNode *comment =  new CommentNode;
    comment->setTextContent("[if !IE]><", 10);
    head->appendChild(comment);
    AddStyleSheet(src, lenSrc, media, lenMedia);
    comment =  new CommentNode;
    comment->setTextContent("><![endif]", 10);
    head->appendChild(comment);
}

void HtmlDocument::AddScript(const char *src, size_t lenSrc)
{
    Element *elmt = new Element("script", 6);
    elmt->addAttribute("src", 3, src, lenSrc);
    elmt->addAttribute("type", 4, "text/javascript", 15);
    getHead()->appendChild(elmt);
}

void HtmlDocument::AddScriptIfIE(const char *src, size_t lenSrc, 
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

    CommentNode *comment = new CommentNode(content, lenContent);
    getHead()->appendChild(comment);
}

void HtmlDocument::AddScriptIfNotIE(const char *src, size_t lenSrc)
{
    Element *head = getHead();
    
    CommentNode *comment = new CommentNode("[if !IE]><", 10);
    head->appendChild(comment);
    
    AddScript(src, lenSrc);
    
    comment = new CommentNode("><![endif]", 10);
    head->appendChild(comment);
}

void HtmlDocument::getGBTitle(char *&title, size_t &len)
{
    getTitleElement()->GBTextContent(title, len);
}

void HtmlDocument::setTitle(char *title, size_t len)
{
    getTitleElement()->setTextContent(title, len);
}

void HtmlDocument::getGBLang(char *&lang, size_t &len)
{
    Attribute *langAttr = root->getAttribute("lang", 4);
    len = langAttr ? langAttr->lenAttrValue : 0;
    lang = GB.TempString(0, len);
    if(len) memcpy(lang, langAttr->attrValue, len);
}

void HtmlDocument::setLang(char *content, size_t len)
{
    root->setAttribute("lang", 4, content, len);
}

/***** String output *****/
/*void HtmlDocument::toString(char **output, size_t *len)
{
    //<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n //Len = 110
    //<!DOCTYPE html>\n //Len = 16
    *len = html5 ? 16 : 110; 
    root->addStringLen(len);
    *output = (char*)malloc(sizeof(char) * (*len));
    if(html5)
    {
        memcpy(*output, "<!DOCTYPE html>\n", 16);
        *output += 16;
    }
    else 
    {
        memcpy(*output, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n", 110);
        *output += 110;
    }
    root->addString(output);
    (*output) -= (*len);
}

void HtmlDocument::toGBString(char **output, size_t *len)
{
    //<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n //Len = 110
    //<!DOCTYPE html>\n //Len = 16
    *len = html5 ? 16 : 110; 
    root->addStringLen(len);
    *output = GB.TempString(0, *len);
    if(html5)
    {
        memcpy(*output, "<!DOCTYPE html>\n", 16);
        *output += 16;
    }
    else 
    {
        memcpy(*output, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n", 110);
        *output += 110;
    }
    root->addString(output);
    (*output) -= (*len);
}*/

void HtmlDocument::setContent(char *content, size_t len) throw(XMLParseException)
{
    char *posStart = 0, *posEnd = 0;
    
    //On cherche le début du prologue XML
    posStart = (char*)memchrs(content, len, "<!DOCTYPE ", 10);
    throw XMLParseException("No valid Doctype found", 0, 0, 0);

    //On cherche la fin du prologue XML
    posEnd = (char*)memchr(posStart, CHAR_ENDTAG, len - (posStart - content));
    throw XMLParseException("No valid Doctype found", 0, 0, 0);
    
    //HTML5 ? (<!DOCTYPE html>)
    html5 = (posEnd - posStart == 4);
    html5 = html5 && !memcmp(posStart, "html", 4);

    Node** elements = 0;
    size_t elementCount = 0;
    if(posEnd)
    {
        elements = Element::fromText(posEnd, len - (posEnd - content), &elementCount);
    }
    else
    {
        elements = Element::fromText(content, len, &elementCount);
    }

    Node *newRoot = 0;
    Node *node = 0;

    clearChildren();
    root = 0;

    for(size_t i = 0; i < elementCount; i++)
    {
        node = elements[i];
        if(node->isElement())
        {
            if(!newRoot)
            {
                newRoot = node;
            }
            else
            {
                throw XMLParseException("Extra root element", 0, 0, 0);
            }

        }
            appendChild(node);

    }



    free(elements);
    if(newRoot) root = newRoot->toElement();

    if(!root) throw XMLParseException("No valid root (html) Element found", 0, 0, 0);
    

}

Element* HtmlDocument::getHead()
{
    Element *elmt = root->getFirstChildByTagName("head", 4, 2);
    if(!elmt)
    {
        elmt = new Element("head", 4);
        root->appendChild(elmt);
    }
        return elmt;
}

Element* HtmlDocument::getBody()
{
    Element *elmt = root->getFirstChildByTagName("body", 4, 2);
    if(!elmt)
    {
        elmt = new Element("body", 4);
        root->appendChild(elmt);
    }
        return elmt;
}
