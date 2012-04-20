#include "node.h"
#include "element.h"
#include "textnode.h"

//ostream &operator<<( ostream &out, Node &node )
//{
//    out << node.toString();
//    return out;
//}

//ostream &operator<<( ostream &out, Node *node )
//{
//    out << *node;
//    return out;
//}

Element* Node::toElement()
{
    if (this->isElement()) return reinterpret_cast<Element*>(this);
    return 0;
}

TextNode* Node::toTextNode()
{
    if (this->isText() || this->isComment() || this->isCDATA()) return reinterpret_cast<TextNode*>(this);
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
    list<Node*>::iterator it;
    for(it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        if(*it == this) break;
    }

    if(it == parent->children->begin()) return 0; //Si c'est le premier, y risque pas d'y en avoir avant

    return *(--(it));

}

Node* Node::next()
{
    if(!parent) return 0;
    list<Node*>::iterator it;
    for(it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        if(*it == this) break;
    }

    if(it == parent->children->end()) return 0; //Si c'est le dernier, y risque pas d'y en avoir après

    return *(++(it));

}
