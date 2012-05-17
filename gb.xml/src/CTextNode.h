#ifndef CTEXTNODE_H
#define CTEXTNODE_H

class TextNode;
class CommentNode;
class CDATANode;
#include "CNode.h"

typedef struct CTextNode
{
    CNode n;
    TextNode *node;
} CTextNode;

typedef struct CCommentNode
{
    CTextNode n;
    CommentNode *node;
} CCommentNode;

typedef struct CCDATANode
{
    CTextNode n;
    CDATANode *node;
} CCDATANode;

#ifndef CLASSES_CPP
extern GB_DESC CTextNodeDesc[];
extern GB_DESC CCommentNodeDesc[];
extern GB_DESC CCDATANodeDesc[];
#endif

#endif // CTEXTNODE_H
