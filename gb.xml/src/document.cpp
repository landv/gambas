#include "document.h"
#include "CDocument.h"

bool Document::NoInstanciate = false;

Document::Document()
{
    root = new Element;
    root->setTagName("xml");
    relob = 0;
    ref = 0;
}

Document::~Document()
{
    //UNREF(relob);
}

GBI::ObjectArray<Node>* Document::getAll()
{
    GBI::ObjectArray<Node> *children = root->getAllChildren();
    children->push_back(root);
    return children;
}

void Document::NewGBObject()
{
    NoInstanciate = true;
    relob = GBI::New<CDocument>("XmlDocument");
    relob->doc = this;
    //GB.Ref(relob);
    NoInstanciate = false;
}

fwstring Document::getContent(bool indent)
{
    fwstring doctype("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    if(indent) doctype += "\n";

    //DEBUGH;
    doctype += root->toString(indent ? 0 : -1);
    return doctype;
}

void Document::setContent(fwstring str)
{
    char *posStart = 0, *posEnd = 0;

    //On cherche le début du prologue XML
    posStart = (char*)memchrs(str.data, str.len, "<?xml ", 6);
    if(!posStart) throw HTMLParseException(0, 0, "nowhere", "No valid XML prolog found.");

    //On cherche la fin du prologue XML
    posEnd = (char*)memchrs(posStart, str.len - (posStart - str.data), "?>", 2);
    if(!posEnd) throw HTMLParseException(0, 0, "nowhere", "No valid XML prolog found.");

    fvector<Node*>* elements = 0;
    try{elements = Element::fromText(str, (posEnd - str.data));}
    catch(HTMLParseException &e) { throw e; }

    //delete root;
    root = 0;
    Node *node = 0;
    //DEBUG << elements->at(0) << " " << elements->data[0] << endl;
    for(size_t i = 0; i < elements->size(); i++)
    {
        //DEBUG << i << endl;
        node = elements->at(i);
        if(node->isElement() && !root)
        {
            root = node->toElement();
            root->setOwnerDocument(this);
        }
        else
        {
            //delete node;
        }

    }

    delete elements;

    if(!root) throw HTMLParseException(0, 0, "somewhere", "No valid root element found.");

}

Element* Document::createElement(fwstring tagName)
{
    Element *elmt = new Element;
    elmt->setTagName(tagName);
    //DEBUG << elmt << endl;
    return elmt;
}

void Document::save(string fileName)
{
    ofstream file((fileName.c_str()), ios::out | ios::trunc);

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
    out << (doc.getContent());
    return out;
}

ostream &operator<<( ostream &out, Document *doc )
{
    out << (doc->getContent());
    return out;
}
