#ifndef NODE_H
#define NODE_H

#include "main.h"
#include "gbi.h"
#include "utils.h"

class Element;
class TextNode;
class CommentNode;
class Document;
class CDATANode;

struct CNode;
class Node
{
public:

    enum Type {BaseNode, ElementNode, NodeText, Comment, CDATA, Attribute};

    Node();
    virtual ~Node();

    //void operator=(const Node& copie);

    virtual Type getType() = 0;
    bool isElement();
    bool isText();
    bool isComment();
    bool isCDATA();

    Element* toElement();
    TextNode* toTextNode();
    CommentNode* toComment();

    void setParent(Element *newparent);
    Element *getParent();

    Node* previous();
    Node* next();

    virtual Node* cloneNode() = 0;

    Document* ownerDocument();
    virtual void setOwnerDocument(Document *doc);

    virtual fwstring toString(int indent = -1) = 0;

    virtual fwstring textContent() = 0;
    virtual void setTextContent(fwstring content) = 0;

    virtual void NewGBObject() = 0;
    virtual CNode* GetGBObject() = 0;

    Element *parent;
    Document *ownerDoc;

    Node *nextNode;
    Node *previousNode;

    //CNode *relob;

    static bool NoInstanciate;
    unsigned char ref;
};
#endif // NODE_H
