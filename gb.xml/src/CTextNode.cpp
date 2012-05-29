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

#include "CTextNode.h"
#include "textnode.h"

#define THISNODE (static_cast<CNode*>(_object)->node)

BEGIN_METHOD(CTextNode_new, GB_STRING content)


if(Node::NoInstanciate) return;
if(GB.Is(_object, GB.FindClass("XmlCommentNode")))//Called as inherited Comment constructor
{
    if(!MISSING(content))
    {
        THISNODE = new CommentNode(STRING(content), LENGTH(content));
    }
    else
    {
        THISNODE = new CommentNode;
    }
}
else if(GB.Is(_object, GB.FindClass("XmlCDATANode")))//Called as inherited CDATA constructor
{
    if(!MISSING(content))
    {
        THISNODE = new CDATANode(STRING(content), LENGTH(content));
    }
    else
    {
        THISNODE = new CDATANode;
    }
}
else 
{
    if(!MISSING(content))
    {
        THISNODE = new TextNode(STRING(content), LENGTH(content));
    }
    else
    {
        THISNODE = new TextNode;
    }
}
        
        THISNODE->GBObject = static_cast<CNode*>(_object);

END_METHOD


GB_DESC CTextNodeDesc[] =
{
    GB_DECLARE("XmlTextNode", sizeof(CNode)), GB_INHERITS("XmlNode"),
    
    GB_METHOD("_new", "", CTextNode_new, "[(Content)s]"),

    GB_END_DECLARE
};

GB_DESC CCommentNodeDesc[] =
{
    GB_DECLARE("XmlCommentNode", sizeof(CNode)), GB_INHERITS("XmlTextNode"),

    GB_END_DECLARE
};

GB_DESC CCDATANodeDesc[] =
{
    GB_DECLARE("XmlCDATANode", sizeof(CNode)), GB_INHERITS("XmlTextNode"),

    GB_END_DECLARE
};

