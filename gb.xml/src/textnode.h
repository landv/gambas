#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "main.h"
#include "node.h"

class TextNode : public Node
{
public:
    class Virtual : public Node::Virtual
    {
    public:
        Virtual(TextNode *node) : Node::Virtual(node), parent(node) {}
        Virtual(const Virtual &copie) : Node::Virtual(copie.parent), parent(copie.parent) {}
        Virtual &operator=(const Virtual &copie) {parent = copie.parent; return *this;}

        virtual Node::Type getType() {return Node::NodeText;}
        virtual wstring toString(int indent = -1);
        virtual wstring textContent() {return *(parent->content);}
        virtual void setTextContent(wstring &content) {*(parent->content) = content;}
        virtual Node* cloneNode();

        TextNode *parent;

    };


    wstring *content;
};

class CommentNode : public TextNode
{
public:
    class Virtual : public TextNode::Virtual
    {
    public:
        Virtual(CommentNode *node) : TextNode::Virtual(node), parent(node) {}
        Virtual(const Virtual &copie) : TextNode::Virtual(copie.parent), parent(copie.parent) {}
        Virtual &operator=(const Virtual &copie) {parent = copie.parent; return *this;}

        virtual Node::Type getType() {return Node::Comment;}
        virtual wstring toString(int indent = -1);
        virtual Node* cloneNode();

        CommentNode *parent;

    };
};


class CDATANode : public TextNode
{
public:
    class Virtual : public TextNode::Virtual
    {
    public:
        Virtual(CDATANode *node) : TextNode::Virtual(node), parent(node) {}
        Virtual(const Virtual &copie) : TextNode::Virtual(copie.parent), parent(copie.parent) {}
        Virtual &operator=(const Virtual &copie) {parent = copie.parent; return *this;}

        virtual Node::Type getType() {return Node::CDATA;}
        virtual wstring toString(int indent = -1);
        virtual Node* cloneNode();

        CDATANode *parent;

    };
};

#endif // TEXTNODE_H
