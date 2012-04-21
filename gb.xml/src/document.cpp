#include "document.h"

GBI::ObjectArray<Node>* Document::getAll()
{
    GBI::ObjectArray<Node> *children = root->getAllChildren();
    children->push_back(root);
    return children;
}

wstring Document::Virtual::getContent(bool indent)
{
    wstring doctype = L"<?xml version=\"1.0\"?>";
    if(indent) doctype += L"\n";

    return doctype + parent->root->toString(indent ? 0 : -1);
}

void Document::setContent(wstring str)
{
    unsigned int i, pos = 0, len = 0, lines = 0;
    wstring prolog, s;
    for (i = 0; i < str.length(); i++)//On cherche le prologue XML
    {
        s = str[i];
        if(str.substr(i, 2) == L"\r\n")
        {
            i++;
            lines++;
        }
        else if((s == L"\n") || (s == L"\r"))
        {
            lines++;
        }
        else if(str.substr(i, 6) == L"<?xml ")//On a trouvé le début du prologue XML
        {
            pos = i;
            i += 6;
            for (; i < str.length(); i++)//On cherche la fin du prologue XML
            {
                if(str.substr(i, 2) == L"?>") {len = i - pos; i +=2; break;}
            }
            if(len) prolog = str.substr(pos, len);
            break;

        }
    }

    if(!prolog.length()) throw HTMLParseException(0, 0, L"somewhere", L"No valid XML prolog found.");

    vector<Node*>* elements = 0;
    try{elements = Element::fromText(str, i, i+1, lines);}
    catch(HTMLParseException &e) { throw e; }

    GB.Unref(POINTER(&root));
    root = 0;
    Node *node = 0;
    for(i = 0; i < elements->size(); i++)
    {
        node = elements->at(i);
        if(node->isElement() && !root)
        {
            root = node->toElement();
            root->setOwnerDocument(this);
            GB.Ref(root);
        }
        else
        {
            GB.Unref(POINTER(&node));
        }

    }

    delete elements;

    if(!root) throw HTMLParseException(0, 0, L"somewhere", L"No valid root element found.");

}

Element* Document::createElement(wstring tagName)
{
    Element *elmt = GBI::New<Element>("XmlElement");
    elmt->setTagName(tagName);
    return elmt;
}

void Document::save(wstring fileName)
{
    ofstream file(WStringToString(fileName), ios::out | ios::trunc);

    if(file)
    {
        file << this;
        file.close();
    }
    else
    {
        GB.Error("Cannot open file");
    }

}

ostream &operator<<( ostream &out, Document &doc )
{
    out << WStringToString(doc.getContent());
    return out;
}

ostream &operator<<( ostream &out, Document *doc )
{
    out << WStringToString(doc->getContent());
    return out;
}
