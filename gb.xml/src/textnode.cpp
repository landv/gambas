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

#include "textnode.h"
#include "utils.h"
#include "gbi.h"
#include "CNode.h"

/*************************************** TextNode ***************************************/

TextNode::TextNode() : Node(), content(0), lenContent(0), escapedContent(0), lenEscapedContent(0)
{
}

TextNode::TextNode(const char *ncontent, const size_t nlen) : Node(), escapedContent(0), lenEscapedContent(0)
{
    lenContent = nlen;
    content = (char*)malloc(sizeof(char) * nlen);
    memcpy(content, ncontent, nlen);
}

TextNode::~TextNode()
{
    if(content) free(content);
    if(escapedContent) free(escapedContent);
}

Node::Type TextNode::getType()
{
    return NodeText;
}

void TextNode::escapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst)
{
    dst = (char*)malloc(lenSrc + 1);
    lenDst = lenSrc + 1;
    dst[lenSrc] = 0;
    memcpy(dst, src, lenSrc);
    char *posFound = strpbrk (dst, "<>&");
    while (posFound != 0)
    {
        switch(*posFound)
        {
        case CHAR_STARTTAG://&lt;
            *posFound = CHAR_AND;
            ++posFound;
            insertString(dst, lenDst , "lt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&");
            break;
            
        case CHAR_ENDTAG: //&gt;
            *posFound = CHAR_AND;
            ++posFound;
            insertString(dst, lenDst, "gt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&");
            break;
            
            
        case CHAR_AND: //&amp;
            *posFound = CHAR_AND;
            ++posFound;
            insertString(dst, lenDst, "amp;", 4, posFound);
            posFound = strpbrk (posFound + 1,"<>&");
            break;
            
        default:
            //posFound = strpbrk (posFound + 1,"<>&");
            break;
        }

      
    }
    
    --lenDst;
}

void TextNode::unEscapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst)
{
    dst = (char*)malloc(lenSrc);
    lenDst = lenSrc;
    memcpy(dst, src, lenSrc);
    char *posFound = (char*)memchr(dst, CHAR_AND, lenDst);
    
    //register bool cond = ((posFound != 0) && ((posFound + 3) < lenDst + dst));
    while(((posFound != 0) && ((posFound + 3) < lenDst + dst)))//(posFound - dst) < lenDst - 3
    //while(cond)//Don't ask ...
    {
        if(memcmp(posFound + 1, "lt;", 3) == 0)// <   &lt;
        {
            *posFound = CHAR_STARTTAG;
            memmove(posFound + 1, posFound + 4, lenDst - (posFound - dst));
            lenDst -= 3;
            posFound -= 3;
        }
        else if(memcmp(posFound + 1, "gt;", 3) == 0)// >   &gt;
        {
            *posFound = CHAR_ENDTAG;
            memmove(posFound + 1, posFound + 4, lenDst - (posFound - dst));
            lenDst -= 3; 
            posFound -= 3;
        }
        else if(memcmp(posFound + 1, "amp;", 4) == 0)// &   &amp;
        {
            memmove(posFound + 1, posFound + 5, lenDst - (posFound - dst));
            lenDst -= 4; 
            posFound -= 4;
        }
        if(posFound + 1 >= dst + lenDst) break;
        posFound = (char*)memchr(posFound + 1, CHAR_AND, lenDst - (posFound + 1 - dst));
        //cond = ((posFound != 0) && ((posFound + 3) < lenDst + dst));
    }
}

void TextNode::checkEscapedContent()
{
    if(!escapedContent && content)
    {
        escapeContent(content, lenContent, escapedContent, lenEscapedContent);
    }
}

void TextNode::checkContent()
{
    if(escapedContent && !content)
    {
        unEscapeContent(escapedContent, lenEscapedContent, content, lenContent);
    }
}

void TextNode::setEscapedTextContent(const char *ncontent, const size_t nlen)
{
    escapedContent = (char*)realloc(escapedContent, sizeof(char)*nlen);
    lenEscapedContent = nlen;
    memcpy(escapedContent, ncontent, nlen);
}


/***** String output *****/

void TextNode::addStringLen(size_t &len, int indent)
{
    checkEscapedContent();
    len += lenEscapedContent;
    if(indent >= 0) len += indent + 1;
}

#undef ADD
#define ADD(_car) *data = _car; ++(data);

void TextNode::addString(char *&data, int indent)
{
    checkEscapedContent();
    if(indent >= 0) 
    {
        memset(data, CHAR_SPACE, indent); 
        data += indent;
    }
    memcpy(data, escapedContent, lenEscapedContent);
    data += lenEscapedContent;
    if(indent >= 0)
    {
        ADD(SCHAR_N);
    }
}

/***** Text Content *****/

void TextNode::setTextContent(const char *ncontent, const size_t nlen)
{
    content = (char*)realloc(content, sizeof(char)*nlen);
    lenContent = nlen;
    memcpy(content, ncontent, nlen);
}

void TextNode::addTextContentLen(size_t &len)
{
    checkContent();
    len += lenContent;
}

void TextNode::addTextContent(char *&data)
{
    checkContent();
    memcpy(data, content, lenContent);
    data += lenContent;
}

/***** Gambas Object *****/

void TextNode::NewGBObject()
{
    NoInstanciate = true;
    GBObject = GBI::New<CNode>("XmlTextNode");
    GBObject->node = this;
    NoInstanciate = false;
}


/*************************************** Comment ***************************************/

CommentNode::CommentNode() : TextNode()
{
    
}


CommentNode::CommentNode(const char *ncontent, const size_t nlen) : TextNode(ncontent, nlen)
{
    
}

CommentNode::~CommentNode()
{
    
}

Node::Type CommentNode::getType()
{
    return Node::Comment;
}

void CommentNode::NewGBObject()
{
    NoInstanciate = true;
    GBObject = GBI::New<CNode>("XmlCommentNode");
    GBObject->node = this;
    NoInstanciate = false;
}

void CommentNode::addStringLen(size_t &len, int indent)
{
    checkEscapedContent();
    // <!-- + content + -->
    len += lenEscapedContent + 7;
    if(indent > 0) len += indent + 1;
}

void CommentNode::addString(char *&data, int indent)
{
    checkEscapedContent();
    if(indent >= 0) 
    {
        memset(data, CHAR_SPACE, indent); 
        data += indent;
    }
    memcpy(data, "<!--", 4);
    data += 4;
    memcpy(data, escapedContent, lenEscapedContent);
    data += lenEscapedContent;
    memcpy(data, "-->", 3);
    data += 3;
    if(indent >= 0)
    {
        ADD(SCHAR_N);
    }
}

/*************************************** CDATA ***************************************/

CDATANode::CDATANode() : TextNode()
{
    
}


CDATANode::CDATANode(const char *ncontent, const size_t nlen) : TextNode(ncontent, nlen)
{
    
}

CDATANode::~CDATANode()
{
    
}

Node::Type CDATANode::getType()
{
    return Node::CDATA;
}

void CDATANode::NewGBObject()
{
    NoInstanciate = true;
    GBObject = GBI::New<CNode>("XmlCDATANode");
    GBObject->node = this;
    NoInstanciate = false;
}

void CDATANode::addStringLen(size_t &len, int indent)
{
    checkEscapedContent();
    // <![CDATA[ + content + ]]>
    len += lenContent + 12;
    if(indent) len += indent + 1;
}

void CDATANode::addString(char *&data, int indent)
{
    checkEscapedContent();
    if(indent >= 0) 
    {
        memset(data, CHAR_SPACE, indent); 
        data += indent;
    }
    memcpy(data, "<![CDATA[", 9);
    data += 9;
    memcpy(data, content, lenContent);
    data += lenContent;
    memcpy(data, "]]>", 3);
    data += 3;
    if(indent >= 0)
    {
        ADD(SCHAR_N);
    }
}
