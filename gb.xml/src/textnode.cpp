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
#include "node.h"

#include <stdlib.h>
#include <memory.h>

/*************************************** TextNode ***************************************/

TextNode* XMLTextNode_New()
{
    TextNode *newTextNode = (TextNode*)malloc(sizeof(TextNode));
    XMLNode_Init(newTextNode, Node::NodeText);
    newTextNode->content = 0;
    newTextNode->lenContent = 0;
    newTextNode->escapedContent = 0;
    newTextNode->lenEscapedContent = 0;

    return newTextNode;
}

TextNode* XMLTextNode_New(const char *ncontent, const size_t nlen)
{
    TextNode *newTextNode = (TextNode*)malloc(sizeof(TextNode));
    XMLNode_Init(newTextNode, Node::NodeText);
    newTextNode->content = 0;
    newTextNode->lenContent = 0;
    newTextNode->escapedContent = 0;
    newTextNode->lenEscapedContent = 0;
    newTextNode->lenContent = nlen;
    if(!newTextNode->lenContent)
    {
        newTextNode->content = 0;
        return newTextNode;
    }
    newTextNode->content = (char*)malloc(sizeof(char) * nlen + 1);
    memcpy(newTextNode->content, ncontent, nlen);
    newTextNode->content[nlen] = 0;

    return newTextNode;
}

void XMLTextNode_Free(TextNode *node)
{
    if(node->escapedContent && node->escapedContent != node->content) free(node->escapedContent);
    if(node->content) free(node->content);
    free(node);
}

void XMLText_escapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst)
{
    dst = (char*)src;
    lenDst = lenSrc;
    if(!lenSrc || !src) return;
    char *posFound = strpbrk (dst, "<>&\"");
    while (posFound != 0)
    {
        if(dst == src)//dst not allocated yet
        {
            dst = (char*)malloc(lenSrc + 1);
            lenDst = lenSrc + 1;
            dst[lenSrc] = 0;
            memcpy(dst, src, lenSrc);
            posFound = ((posFound - src) + dst);
        }
        switch(*posFound)
        {
        case '<'://&lt;
            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst , "lt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"");
            break;
            
        case '>': //&gt;
            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "gt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"");
            break;
            
            
        case '&': //&amp;

            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "amp;", 4, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"");
            break;

        case '"': //&quot;

            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "quot;", 5, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"");
            break;
            
        default:
            //posFound = strpbrk (posFound + 1,"<>&");
            break;
        }

      
    }
    
    if(dst != src) --lenDst;
}

void XMLText_escapeAttributeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst)
{
    dst = (char*)src;
    lenDst = lenSrc;
    if(!lenSrc || !src) return;
    char *posFound = strpbrk (dst, "<>&\"\n");
    while (posFound != 0)
    {
        if(dst == src)//dst not allocated yet
        {
            dst = (char*)malloc(lenSrc + 1);
            lenDst = lenSrc + 1;
            dst[lenSrc] = 0;
            memcpy(dst, src, lenSrc);
            posFound = ((posFound - src) + dst);
        }
        switch(*posFound)
        {
        case '<'://&lt;
            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst , "lt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"\n");
            break;

        case '>': //&gt;
            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "gt;", 3, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"\n");
            break;


        case '&': //&amp;

            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "amp;", 4, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"\n");
            break;

        case '"': //&quot;

            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "quot;", 5, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"\n");
            break;

        case '\n':
            *posFound = '&';
            ++posFound;
            insertString(dst, lenDst, "#10;", 4, posFound);
            posFound = strpbrk (posFound + 1,"<>&\"\n");
            break;

        default:
            //posFound = strpbrk (posFound + 1,"<>&");
            break;
        }


    }

    if(dst != src) --lenDst;
}

void XMLText_unEscapeContent(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst)
{
    dst = (char*)malloc(lenSrc);
    lenDst = lenSrc;
    memcpy(dst, src, lenSrc);
    char *posFound = (char*)memchr(dst, '&', lenDst);

    while(((posFound != 0) && ((posFound + 3) < lenDst + dst)))//(posFound - dst) < lenDst - 3
    {
        if(memcmp(posFound + 1, "lt;", 3) == 0)// <   &lt;
        {
            *posFound = '<';
            //.......dst=========posFound!===posFound+x|======dst+lenDst............
            //lenCut = (pos2 - pos1) = (dst + lenDst) - (posFound + x)
            memmove(posFound + 1, posFound + 4, (dst + lenDst) - (posFound + 4));
            lenDst -= 3;
            posFound -= 3;
        }
        else if(memcmp(posFound + 1, "gt;", 3) == 0)// >   &gt;
        {
            *posFound = '>';
            memmove(posFound + 1, posFound + 4, (dst + lenDst) - (posFound + 4));
            lenDst -= 3; 
            posFound -= 3;
        }
        else if(((posFound + 4) < lenDst + dst) && memcmp(posFound + 1, "amp;", 4) == 0)// &   &amp;
        {
            memmove(posFound + 1, posFound + 5, (dst + lenDst) - (posFound + 5));
            lenDst -= 4; 
            posFound -= 4;
        }
        else if(((posFound + 5) < lenDst + dst) && memcmp(posFound + 1, "quot;", 5) == 0)// "&"   &quot;
        {
            *posFound = '"';
            memmove(posFound + 1, posFound + 6, (dst + lenDst) - (posFound + 6));
            lenDst -= 5;
            posFound -= 5;
        }
        if(posFound + 1 >= dst + lenDst) break;
        posFound = (char*)memchr(posFound + 1, '&', lenDst - (posFound + 1 - dst));
        //cond = ((posFound != 0) && ((posFound + 3) < lenDst + dst));
    }
}

void XMLTextNode_checkEscapedContent(TextNode *node)
{
    if(!(node->escapedContent) && node->content)
    {
        XMLText_escapeContent(node->content, node->lenContent, node->escapedContent, node->lenEscapedContent);
    }
}

void XMLTextNode_checkContent(TextNode *node)
{
    if(node->escapedContent && !(node->content))
    {
        XMLText_unEscapeContent(node->escapedContent, node->lenEscapedContent, node->content, node->lenContent);
    }
}

void XMLTextNode_setEscapedTextContent(TextNode *node, const char *ncontent, const size_t nlen)
{
    node->escapedContent = (char*)realloc(node->escapedContent, sizeof(char)*nlen);
    node->lenEscapedContent = nlen;
    memcpy(node->escapedContent, ncontent, nlen);
}

/***** Text Content *****/

void XMLTextNode_TrimContent(TextNode *node)
{
    const char *oldcontent = node->content;
    Trim(oldcontent, node->lenContent);
    memmove(node->content, oldcontent, node->lenContent);
    node->content = (char*)realloc(node->content, sizeof(char)*node->lenContent);
}

void XMLTextNode_setTextContent(TextNode *node, const char *ncontent, const size_t nlen)
{
    node->content = (char*)realloc(node->content, sizeof(char)*nlen + 1);
    node->lenContent = nlen;
    memcpy(node->content, ncontent, nlen);
    node->content[node->lenContent] = 0;
}

/*************************************** Comment ***************************************/

CommentNode* XMLComment_New()
{
    CommentNode *newComment = XMLTextNode_New();
    newComment->type = Node::Comment;
    return newComment;
}


CommentNode* XMLComment_New(const char *ncontent, const size_t nlen)
{
    CommentNode *newComment = XMLTextNode_New(ncontent, nlen);
    newComment->type = Node::Comment;
    return newComment;
}

/*************************************** CDATA ***************************************/

CDATANode* XMLCDATA_New()
{
    CommentNode *newComment = XMLTextNode_New();
    newComment->type = Node::Comment;
    return newComment;
}

CDATANode* XMLCDATA_New(const char *ncontent, const size_t nlen)
{
    CommentNode *newComment = XMLTextNode_New(ncontent, nlen);
    newComment->type = Node::Comment;
    return newComment;
}

