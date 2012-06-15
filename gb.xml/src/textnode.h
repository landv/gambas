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

#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "main.h"
#include "node.h"

class TextNode : public Node
{
public:
    TextNode();
    TextNode(const char *ncontent, const size_t nlen);
    virtual ~TextNode();
    virtual Node::Type getType();
    
    //static void *operator new (size_t taille, TextNode *other);
    //static void *operator new (size_t taille);
    
    static void unEscapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
    static void escapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
    void checkEscapedContent();
    void checkContent();
    void setEscapedTextContent(const char *ncontent, const size_t nlen);
    
    //String output
    virtual void addStringLen(size_t *len, int indent = 0);
    virtual void addString(char **data, int indent = 0);
    
    //Text content
    virtual void setTextContent(const char *ncontent, const size_t nlen);
    virtual void addTextContentLen(size_t &len);
    virtual void addTextContent(char *&data);
    char *content;
    size_t lenContent;
    
    char *escapedContent;
    size_t lenEscapedContent;
    
    //Gambas object
    virtual void NewGBObject();
};


class CommentNode : public TextNode
{
public:
    CommentNode();
    CommentNode(const char *ncontent, const size_t nlen);
    virtual ~CommentNode();
    virtual Node::Type getType();
    
    //String output
    virtual void addStringLen(size_t *len, int indent = 0);
    virtual void addString(char **data, int indent = 0);
    
    //Gambas object
    virtual void NewGBObject();
};


class CDATANode : public TextNode
{
public:
    CDATANode();
    CDATANode(const char *ncontent, const size_t nlen);
    virtual ~CDATANode();
    virtual Node::Type getType();
    
    //String output
    virtual void addStringLen(size_t *len, int indent = 0);
    virtual void addString(char **data, int indent = 0);
    
    //Gambas object
    virtual void NewGBObject();
};


#endif // TEXTNODE_H
