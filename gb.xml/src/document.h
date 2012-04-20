#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "main.h"
#include "element.h"
#include <fstream>

class Document;

ostream &operator<<( ostream &out, Document &doc );
ostream &operator<<( ostream &out, Document *doc );

class Document : public GB_BASE
{
public:
    void operator=(const Document& copie);

    class Virtual
    {
    public:
        Virtual(Document *doc) : parent(doc) {}
        Virtual(const Document::Virtual &copie) :  parent(copie.parent) {}
        Document::Virtual &operator=(const Document::Virtual &copie) {parent = copie.parent; return *this;}
        virtual wstring getContent(bool indent = false);

        Document *parent;

    };

    Element* getRoot() { return root; }
    void setRoot(Element* newRoot) {root = newRoot;}

    GBI::ObjectArray<Element>* getGBElementsByTagName(wstring tag, int depth = -1) {return root->getGBChildrenByTagName(tag, depth);}
    vector<Element*>* getElementsByTagName(wstring tag, int depth = -1) {return root->getChildrenByTagName(tag, depth);}
    GBI::ObjectArray<Node>* getAll();
    Element* createElement(wstring tagName);

    wstring getContent(bool indent = false){return virt->getContent(indent);}
    void setContent(wstring str);
    void save(wstring fileName);

    Element *root;

    Virtual *virt;

};


#endif // DOCUMENT_H
