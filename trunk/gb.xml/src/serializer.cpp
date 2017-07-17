#include "serializer.h"

#include "utils.h"
#include "node.h"
#include "textnode.h"

#include "gbinterface.h"

#include <stdlib.h>

void serializeNode(Node *node, char *&output, size_t &len, int indent)
{
    Document *parentDocument = XMLNode_GetOwnerDocument(node);
    if(parentDocument)
    {
        if(parentDocument->docType == HTMLDocumentType || parentDocument->docType == XHTMLDocumentType)
        {
            if(CheckHtmlInterface())
            {
                HTML.serializeHTMLNode(node, output, len, indent);
                return;
            }
            else
            {
                //DEBUG << "WARNING : HTML Serializer not found" << endl;
            }
        }
    }

        serializeXMLNode(node, output, len, indent);
}

void GBserializeNode(Node *node, char *&output, size_t &len, int indent)
{
    Document *parentDocument = XMLNode_GetOwnerDocument(node);
    if(parentDocument)
    {
        if(parentDocument->docType == HTMLDocumentType || parentDocument->docType == XHTMLDocumentType)
        {
            if(CheckHtmlInterface())
            {
                HTML.GBserializeHTMLNode(node, output, len, indent);
                return;
            }
            else
            {
                //DEBUG << "WARNING : HTML Serializer not found" << endl;
            }
        }
    }

        GBserializeXMLNode(node, output, len, indent);
}


/***** String output *****/

void addStringLen(Node *node, size_t &len, int indent = -1);//Calculates the node's string representation length, and adds it to len (recursive)
void addString(Node *node, char *&data, int indent = -1);//Puts the string represenetation into data, and increments it (recursive)

void serializeXMLNode(Node *node, char *&output, size_t &len, int indent)
{
    len = 0;
    addStringLen(node, len, indent);
    output = (char*)malloc(sizeof(char) * (len));
    addString(node, output, indent);
    output -= len;

}

void GBserializeXMLNode(Node *node, char *&output, size_t &len, int indent)
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
        len += 38 + (indent >= 0 ? 1 : 0);// root->addStringLen(len, indent);
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
        /*if(((Element*)node)->isSingle())
        {
            len += (4 + ((Element*)node)->lenTagName);
            if(indent >= 0) len += indent + 1;
        }
        else
        {*/
            len += (5 + ((((Element*)node)->lenTagName) * 2));
            if(indent >= 0) len += indent * 2 + 2;
            for(Node *child = node->firstChild; child != 0; child = child->nextNode)
            {
                addStringLen(child, len, indent >= 0 ? indent + 1 : -1);
            }
        //}

        for(Attribute *attr = (Attribute*)(((Element*)node)->firstAttribute); attr != 0; attr = (Attribute*)(attr->nextNode))
        {
            len += 4 + attr->lenAttrName + attr->lenAttrValue;
        }
        break;
    case Node::NodeText:

        XMLTextNode_checkEscapedContent((TextNode*)node);
        len += ((TextNode*)node)->lenEscapedContent;
        if(indent >= 0) len += indent + 1;
        break;

    case Node::Comment:

        XMLTextNode_checkEscapedContent((TextNode*)node);
        // <!-- + content + -->
        len += ((TextNode*)node)->lenEscapedContent + 7;
        if(indent >= 0) len += indent + 1;
        break;
    case Node::CDATA:

        XMLTextNode_checkEscapedContent((TextNode*)node);
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
    //bool single;
    switch (node->type)
    {
    case Node::DocumentNode:
        memcpy(data, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 38);
        data += 38;
        if(indent >= 0)
        {
            *data = '\n';
            ++data;
        }
        //Content
        for(register Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            addString(child, data, indent >= 0 ? indent : -1);
        }
        break;
    case Node::ElementNode:
        //register char *content = data;
        //single = ((Element*)node)->isSingle();

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

        /*if(single)
        {
            ADD(CHAR_SPACE);
            ADD(CHAR_SLASH);
        }*/
        ADD('>');
        if(indent >= 0) { ADD('\n'); }

        //if(!single)
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
        XMLTextNode_checkEscapedContent((TextNode*)node);
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
        XMLTextNode_checkEscapedContent((TextNode*)node);
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
        XMLTextNode_checkEscapedContent((TextNode*)node);
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

/***** Text Content *****/
void addTextContentLen(Node *node, size_t &len);
void addTextContent(Node *node, char *&data);

void GetXMLTextContent(Node *node, char *&output, size_t &len)
{
    len = 0;
    addTextContentLen(node, len);
    output = (char*)malloc(sizeof(char) * (len));
    addTextContent(node, output);
    output -= len;
}

void GBGetXMLTextContent(Node *node, char *&output, size_t &len)
{
    len = 0;
    addTextContentLen(node, len);
    output = GB.TempString(0, len);
    addTextContent(node, output);
    output -= len;
}

void addTextContentLen(Node *node, size_t &len)
{
    if(!node) return;
    switch(node->type)
    {
    case Node::DocumentNode:
    case Node::ElementNode:
        for(Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            addTextContentLen(child, len);
        }
        break;
    case Node::NodeText:
    case Node::CDATA:
        XMLTextNode_checkContent((TextNode*)node);
        len += ((TextNode*)node)->lenContent;
        break;
    case Node::AttributeNode:
        len += ((Attribute*)node)->lenAttrValue;
        break;
    default:
        break;
    }
}

void addTextContent(Node *node, char *&data)
{
    if(!node) return;
    switch(node->type)
    {
    case Node::DocumentNode:
    case Node::ElementNode:
        for(Node *child = node->firstChild; child != 0; child = child->nextNode)
        {
            addTextContent(child, data);
        }
        break;
    case Node::NodeText:
    case Node::CDATA:
        memcpy(data, ((TextNode*)node)->content, ((TextNode*)node)->lenContent);
        data += ((TextNode*)node)->lenContent;
        break;
    case Node::AttributeNode:
        memcpy(data, ((Attribute*)node)->attrValue, ((Attribute*)node)->lenAttrValue);
        data += ((Attribute*)node)->lenAttrValue;
        break;
    default:
        break;
    }
}
