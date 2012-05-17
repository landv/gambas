#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "main.h"
#include "node.h"

struct CTextNode;
struct CCDATANode;
struct CCommentNode;

class TextNode : public Node
{
public:
    TextNode();
    virtual ~TextNode();

    virtual fwstring toString(int indent = -1);
    virtual Node* cloneNode();
    virtual Node::Type getType();
    virtual void setTextContent(fwstring newcontent);

    virtual void NewGBObject();
    virtual CNode* GetGBObject(){return (CNode*)relElmt;}
    virtual fwstring textContent();

    fwstring *content;
    CTextNode *relElmt;
};

class CommentNode : public TextNode
{
public:
    virtual fwstring toString(int indent);
    virtual Node* cloneNode();
    virtual Node::Type getType();
    virtual CNode* GetGBObject(){return (CNode*)relNode;}

    virtual void NewGBObject();
    CCommentNode *relNode;
};


class CDATANode : public TextNode
{
public:
    virtual fwstring toString(int indent);
    virtual Node* cloneNode();
    virtual Node::Type getType();

    virtual void NewGBObject();
    virtual CNode* GetGBObject(){return (CNode*)relNode;}

    CCDATANode *relNode;

};

#endif // TEXTNODE_H
