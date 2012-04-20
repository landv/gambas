#ifndef NODE_H
#define NODE_H

#include "main.h"
class Element;
class TextNode;
class CommentNode;
class Document;
class CDATANode;

class Node : public GB_BASE
{
public:
    void operator=(const Node& copie);
    enum Type {BaseNode, ElementNode, NodeText, Comment, CDATA} ;

    class Virtual
    {
    public:
        Virtual(Node *node):parent(node) {}
        Virtual(const Node::Virtual &copie) : parent(copie.parent) {}
        Node::Virtual &operator=(const Node::Virtual &copie) {parent = copie.parent; return *this;}
        virtual ~Virtual() {}
        virtual Node::Type getType() {return Node::BaseNode;}
        virtual wstring toString(int indent = -1) = 0;
        virtual wstring textContent() = 0;
        virtual void setTextContent(wstring content) = 0;
        virtual void setOwnerDocument(Document *doc) {parent->ownerDoc = doc;}
        virtual Node* cloneNode() = 0;
        Node *parent;
    };

    Type getType(){return virt->getType();}
    bool isElement() {return (getType() == Node::ElementNode);}
    bool isText() {return (getType() == Node::NodeText || getType() == Node::Comment || getType() == Node::CDATA);}
    bool isComment() {return (getType() == Node::Comment);}
    bool isCDATA() {return (getType() == Node::CDATA);}
    void setParent(Element *newparent){parent = newparent;}
    Element *getParent(){return parent;}

    Node* previous();
    Node* next();

    Element* toElement();
    TextNode* toTextNode();
    CommentNode* toComment();
    Node* cloneNode();

    Document* ownerDocument(){return ownerDoc;}
    void setOwnerDocument(Document *doc){virt->setOwnerDocument(doc);}
    wstring toString(int indent = -1) {return virt->toString(indent);}
    wstring textContent() {return virt->textContent();}
    void setTextContent(wstring content) {virt->setTextContent(content);}

    Element *parent;
    Document *ownerDoc;
    Virtual *virt;
};

#include "element.h"

//ostream &operator<<( ostream &out, Node &node );
//ostream &operator<<( ostream &out, Node *node );

#endif // NODE_H
