#include "textnode.h"

wstring TextNode::Virtual::toString(int indent)
{
    wstring str, s;
    if(indent > 0){str += wstring(indent, ' ');};

    for(unsigned int i = 0; i < parent->content->length(); i++)
    {
        s = parent->content->at(i);
        if(s == L"<")
        {
            str += L"&lt;";
        }
        else if(s == L">")
        {
            str += L"&gt;";
        }
        else if(s == L"&")
        {
            /*if((parent->content->substr(i, 6) == L"&nbsp;"))
            {str+= L"&nbsp;"; i +=5;} //On ignore les espaces
            else */str += L"&amp;";
        }
        else if(s == L"\"")
        {
            str += L"&quot;";
        }
        else if(s == L"\n" && indent > 0)
        {
            str += wstring(indent, ' ');
            str += s;
        }
        else
        {
            str += s;
        }
    }


    if(indent >= 0) str += L"\n";

    return str;
}

Node* TextNode::Virtual::cloneNode()
{
    TextNode* node = GBI::New<TextNode>("XmlTextNode");
    node->setTextContent(*(parent->content));
    return node;
}

wstring CommentNode::Virtual::toString(int indent)
{
    wstring str, s;
    str = L"<!--";
    for(unsigned int i = 0; i < parent->content->length(); i++)
    {
        s = parent->content->at(i);
        if((i + 2) < parent->content->length())
        {
            if(parent->content->substr(i, 3) == L"-->") {str += L"--&gt;"; i += 2;}
            else str += s;
        }
        else
        {
            str += s;
        }
    }

    str += L"-->";
    if(indent) str += L"\n";

    return str;
}

Node* CommentNode::Virtual::cloneNode()
{
    CommentNode* node = GBI::New<CommentNode>("XmlCommentNode");
    node->setTextContent(*(parent->content));
    return node;
}

wstring CDATANode::Virtual::toString(int indent)
{
    wstring str, s;
    str = L"<![CDATA[";
    for(unsigned int i = 0; i < parent->content->length(); i++)
    {
        s = parent->content->at(i);
        if((i + 2) < parent->content->length())
        {
            if(parent->content->substr(i, 3) == L"]]>") {str += L"]]&gt;"; i += 2;}
            else str += s;
        }
        else
        {
            str += s;
        }
    }

    str += L"]]>";
    if(indent) str += L"\n";

    return str;
}

Node* CDATANode::Virtual::cloneNode()
{
    CDATANode* node = GBI::New<CDATANode>("XmlCDATANode");
    node->setTextContent(*(parent->content));
    return node;
}
