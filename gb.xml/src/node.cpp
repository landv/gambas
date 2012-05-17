#include "node.h"
#include "element.h"
#include "textnode.h"
#include "CNode.h"

bool Node::NoInstanciate = false;

Node::Node()
{
    ownerDoc = 0;
    parent = 0;
    ref = 0;
    nextNode = 0;
    previousNode = 0;
}

Node::~Node()
{

}

bool Node::isElement()
{
    return getType() == Node::ElementNode;
}

bool Node::isText()
{
    return (getType() == Node::NodeText) ||
           (getType() == Node::Comment) ||
           (getType() == Node::CDATA);
}

bool Node::isComment()
{
    return getType() == Node::Comment;
}

bool Node::isCDATA()
{
    return getType() == Node::CDATA;
}

Element* Node::toElement()
{
    if (this->isElement()) return reinterpret_cast<Element*>(this);
    return 0;
}

TextNode* Node::toTextNode()
{
    if (this->isText()) return reinterpret_cast<TextNode*>(this);
    return 0;
}

CommentNode* Node::toComment()
{
    if(this->isComment()) return reinterpret_cast<CommentNode*>(this);
    return 0;
}

Node* Node::previous()
{
    if(!parent) return 0;
    return previousNode;

}

Node* Node::next()
{
    if(!parent) return 0;
    return nextNode;
}

/*void Node::NewGBObject()
{
    NoInstanciate = true;
    relob = GBI::New<CNode>("XmlNode");
    relob->node = this;
    //GB.Ref(relob);
    NoInstanciate = false;
}*/

void Node::setParent(Element *newparent)
{
    parent = newparent;
}

Element* Node::getParent()
{
    return parent;
}

Document* Node::ownerDocument()
{
    return ownerDoc;
}

void Node::setOwnerDocument(Document *doc)
{
    ownerDoc = doc;
}
