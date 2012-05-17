#include "textnode.h"
#include "element.h"

#include "CTextNode.h"

/****************TextNode****************/

TextNode::TextNode()
{
    content = new fwstring();
}
TextNode::~TextNode()
{

}

Node::Type TextNode::getType()
{
    return Node::NodeText;
}

void TextNode::NewGBObject()
{
    NoInstanciate = true;
    relElmt = GBI::New<CTextNode>("XmlTextNode");
    relElmt->n.node = this;
    relElmt->node = this;
    //GB.Ref(relob);
    NoInstanciate = false;
}

fwstring TextNode::textContent()
{
    return *content;
}

void TextNode::setTextContent(fwstring newcontent)
{
    DELETE(content);
    content = new fwstring(newcontent);
}

fwstring TextNode::toString(int indent)
{
    /*fwstring str, s;
    if(indent > 0){str += fwstring(indent, ' ');};

    for(unsigned int i = 0; i < content->len; i++)
    {
        s = content->data[i];
        if(s == "<")
        {
            str += "&lt;";
        }
        else if(s == ">")
        {
            str += "&gt;";
        }
        else if(s == "&")
        {
            if((parent->content->substr(i, 6) == "&nbsp;"))
            {str+= "&nbsp;"; i +=5;} //On ignore les espaces
            else * /str += "&amp;";
        }
        else if(s == "\"")
        {
            str += "&quot;";
        }
        else if(s == "\n" && indent > 0)
        {
            str += fwstring(indent, ' ');
            str += s;
        }
        else
        {
            str += s;
        }
    }


    if(indent >= 0) str += "\n";

    return str;*/
    return *content;
}

Node* TextNode::cloneNode()
{
    TextNode* node = new TextNode;
    node->setTextContent(content->toString());
    return node;
}

/****************CommentNode****************/

fwstring CommentNode::toString(int indent)
{
    fwstring str, s;
    str = "<!--";
    for(unsigned int i = 0; i < content->len; i++)
    {
        s = content->data[i];
        /*if((i + 2) < parent->content->len)
        {
            if(parent->content->substr(i, 3) == "-->") {str += "--&gt;"; i += 2;}
            else str += s;
        }
        else
        {*/
            str += s;
        //}
    }

    str += "-->";
    if(indent) str += "\n";

    return str;
}

Node* CommentNode::cloneNode()
{
    CommentNode* node = new CommentNode;
    node->setTextContent(content->toString());
    return node;
}

void CommentNode::NewGBObject()
{
    NoInstanciate = true;
    relNode = GBI::New<CCommentNode>("XmlCommentNode");
    relNode->node = this;
    relNode->n.node = this;
    relNode->n.n.node = this;
    //GB.Ref(relob);
    NoInstanciate = false;;
}

Node::Type CommentNode::getType()
{
    return Node::Comment;
}

/****************CDATANode****************/

fwstring CDATANode::toString(int indent)
{
    fwstring str, s;
    str = "<![CDATA[";
    for(unsigned int i = 0; i < content->len; i++)
    {
        s = content->data[i];
        if((i + 2) < content->len)
        {
            /*if(parent->content->substr(i, 3) == "]]>") {str += "]]&gt;"; i += 2;}
            else */str += s;
        }
        else
        {
            str += s;
        }
    }

    str += "]]>";
    if(indent) str += "\n";

    return str;
}

Node* CDATANode::cloneNode()
{
    CDATANode* node = new CDATANode;
    node->setTextContent(*(content));
    return node;
}

Node::Type CDATANode::getType()
{
    return Node::CDATA;
}

void CDATANode::NewGBObject()
{
    NoInstanciate = true;
    relNode = GBI::New<CCDATANode>("XmlCDATANode");
    relNode->node = this;
    relNode->n.node = this;
    relNode->n.n.node = this;
    //GB.Ref(relob);
    NoInstanciate = false;
}
