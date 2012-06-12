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

TextNode::TextNode() : Node()
{
    content = 0;
    lenContent = 0;
}

TextNode::TextNode(const char *ncontent, const size_t nlen) : Node()
{
    lenContent = nlen;
    content = (char*)malloc(sizeof(char) * nlen);
    memcpy(content, ncontent, nlen);
}

TextNode::~TextNode()
{
    if(content) free(content);
}

Node::Type TextNode::getType()
{
    return NodeText;
}


/***** String output *****/

void TextNode::addStringLen(size_t *len, int indent)
{
    *len += lenContent;
    if(indent) *len += indent + 1;
}

#undef ADD
#define ADD(_car) **data = _car; ++(*data);

void TextNode::addString(char **data, int indent)
{
    if(indent >= 0) 
    {
        memset(*data, CHAR_SPACE, indent); 
        *data += indent;
    }
    memcpy(*data, content, lenContent);
    *data += lenContent;
    if(indent >= 0)
    {
        ADD(SCHAR_N);
    }
}

/***** Text Content *****/

void TextNode::setTextContent(const char *ncontent, const size_t nlen)
{
    if(!ncontent) 
    {
        content = (char*)malloc(sizeof(char)*nlen);
    }
    else
    {
        content = (char*)realloc(content, sizeof(char)*nlen);
    }
}

void TextNode::addTextContentLen(size_t &len)
{
    len += lenContent;
}

void TextNode::addTextContent(char *&data)
{
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

void CommentNode::addStringLen(size_t *len, int indent)
{
    // <!-- + content + -->
    *len += lenContent + 7;
    if(indent > 0) *len += indent + 1;
}

void CommentNode::addString(char **data, int indent)
{
    if(indent >= 0) 
    {
        memset(*data, CHAR_SPACE, indent); 
        *data += indent;
    }
    memcpy(*data, "<!--", 4);
    *data += 4;
    memcpy(*data, content, lenContent);
    *data += lenContent;
    memcpy(*data, "-->", 3);
    *data += 3;
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

void CDATANode::addStringLen(size_t *len, int indent)
{
    // <![CDATA[ + content + ]]>
    *len += lenContent + 12;
    if(indent) *len += indent + 1;
}

void CDATANode::addString(char **data, int indent)
{
    if(indent >= 0) 
    {
        memset(*data, CHAR_SPACE, indent); 
        *data += indent;
    }
    memcpy(*data, "<![CDATA[", 9);
    *data += 9;
    memcpy(*data, content, lenContent);
    *data += lenContent;
    memcpy(*data, "]]>", 3);
    *data += 3;
    if(indent >= 0)
    {
        ADD(SCHAR_N);
    }
}
