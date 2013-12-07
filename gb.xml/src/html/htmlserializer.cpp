#include "htmlserializer.h"
#include "htmlelement.h"
#include "../gbinterface.h"
#include <stdlib.h>

void addStringLen(Node *node, size_t &len, int indent = -1);//Calculates the node's string representation length, and adds it to len (recursive)
void addString(Node *node, char *&data, int indent = -1);//Puts the string represenetation into data, and increments it (recursive)


void serializeHTMLNode(Node *node, char *&output, size_t &len, int indent)
{
    len = 0;
    addStringLen(node, len, indent);
    output = (char*)malloc(sizeof(char) * (len));
    addString(node, output, indent);
    output -= len;

}

void GBserializeHTMLNode(Node *node, char *&output, size_t &len, int indent)
{
    len = 0;
    addStringLen(node, len, indent);
    output = GB.TempString(0, len);
    addString(node, output, indent);
    output -= len;
}

void addStringLen(Node *node, size_t &len, int indent)
{
    switch (node->type)
    {
    case Node::DocumentNode:
        len += 15 + (indent >= 0 ? 1 : 0);// root->addStringLen(len, indent);
        //Content
        for(register Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            addStringLen(child, len, indent >= 0 ? indent : -1);
        }
        break;
    case Node::ElementNode:
        // (indent) '<' + prefix:tag + (' ' + attrName + '=' + '"' + attrValue + '"') + '>' \n
        // + children + (indent) '</' + tag + '>" \n
        // Or, singlElement :
        // (indent) '<' + prefix:tag + (' ' + attrName + '=' + '"' + attrValue + '"') + ' />' \n
        if(HTMLElement_IsSingle((Element*)node))
        {
            len += (4 + ((Element*)node)->lenTagName);
            if(indent >= 0) len += indent + 1;
        }
        else
        {
            len += (5 + ((((Element*)node)->lenTagName) * 2));
            if(indent >= 0) len += indent * 2 + 2;
            for(Node *child = node->firstChild; child != 0; child = child->nextNode)
            {
                addStringLen(child, len, indent >= 0 ? indent + 1 : -1);
            }
        }

        for(Attribute *attr = (Attribute*)(((Element*)node)->firstAttribute); attr != 0; attr = (Attribute*)(attr->nextNode))
        {
            len += 4 + attr->lenAttrName + attr->lenAttrValue;
        }
        break;
    case Node::NodeText:

        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        len += ((TextNode*)node)->lenEscapedContent;
        if(indent >= 0) len += indent + 1;
        break;

    case Node::Comment:

        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        // <!-- + content + -->
        len += ((TextNode*)node)->lenEscapedContent + 7;
        if(indent >= 0) len += indent + 1;
        break;
    case Node::CDATA:

        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        // <![CDATA[ + content + ]]>
        len += ((TextNode*)node)->lenContent + 12;
        if(indent) len += indent + 1;
        break;

    default:
        break;
    }
}


#define ADD(_car) *data = _car; data++;
void addString(Node *node, char *&data, int indent)
{
    bool single;
    switch (node->type)
    {
    case Node::DocumentNode:
        memcpy(data, "<!DOCTYPE html>", 15);
        data += 15;
        if(indent >= 0)
        {
            ADD('\n')
        }
        //Content
        for(register Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            addString(child, data, indent >= 0 ? indent : -1);
        }
        break;
    case Node::ElementNode:
        //register char *content = data;
        single = HTMLElement_IsSingle((Element*)node);

        //Opening tag
        if(indent > 0)
        {
            memset(data, ' ', indent);
            data += indent;
        }
        ADD('<');
        memcpy(data, ((Element*)node)->tagName, ((Element*)node)->lenTagName); data += ((Element*)node)->lenTagName;

        //Attributes
        for(register Attribute *attr = (Attribute*)((Element*)node)->firstAttribute; attr != 0; attr = (Attribute*)(attr->nextNode))
        {
            ADD(' ');
            memcpy(data, attr->attrName, attr->lenAttrName); data += attr->lenAttrName;

            ADD('=');
            ADD('"');
            memcpy(data, attr->attrValue, attr->lenAttrValue); data += attr->lenAttrValue;
            ADD('"');
        }

        if(single)
        {
            ADD(' ');
            ADD('/');
        }
        ADD('>');
        if(indent >= 0) { ADD('\n'); }

        if(!single)
        {

            //Content
            for(register Node *child = ((Element*)node)->firstChild; child != 0; child = child->nextNode)
            {
                addString(child, data, indent >= 0 ? indent + 1 : -1);
            }

            if(indent > 0)
            {
                memset(data, ' ', indent);
                data += indent;
            }

            //Ending Tag
            ADD('<');
            ADD('/');
            memcpy(data, ((Element*)node)->tagName, ((Element*)node)->lenTagName); data += ((Element*)node)->lenTagName;
            ADD('>');
            if(indent >= 0) { ADD('\n'); }

        }
        break;
    case Node::NodeText:
        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        if(indent >= 0)
        {
            memset(data, ' ', indent);
            data += indent;
        }

        memcpy(data, ((TextNode*)node)->escapedContent, ((TextNode*)node)->lenEscapedContent);
        data += ((TextNode*)node)->lenEscapedContent;
        if(indent >= 0)
        {
            ADD('\n');
        }
        break;
    case Node::Comment:
        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        if(indent >= 0)
        {
            memset(data, ' ', indent);
            data += indent;
        }
        memcpy(data, "<!--", 4);
        data += 4;
        memcpy(data, ((CommentNode*)node)->escapedContent, ((CommentNode*)node)->lenEscapedContent);
        data += ((CommentNode*)node)->lenEscapedContent;
        memcpy(data, "-->", 3);
        data += 3;
        if(indent >= 0)
        {
            ADD('\n');
        }
        break;
    case Node::CDATA:
        XML.XMLTextNode_checkEscapedContent((TextNode*)node);
        if(indent >= 0)
        {
            memset(data, ' ', indent);
            data += indent;
        }
        memcpy(data, "<![CDATA[", 9);
        data += 9;
        memcpy(data, ((CDATANode*)node)->content, ((CDATANode*)node)->lenContent);
        data += ((CDATANode*)node)->lenContent;
        memcpy(data, "]]>", 3);
        data += 3;
        if(indent >= 0)
        {
            ADD('\n');
        }
        break;
    default:
        break;
    }
}
