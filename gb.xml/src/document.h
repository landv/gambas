#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "main.h"
#include "element.h"
#include <fstream>

class Document;
struct CDocument;

ostream &operator<<( ostream &out, Document &doc );
ostream &operator<<( ostream &out, Document *doc );

class Document
{
public:
    Document();
    ~Document();
    void operator=(const Document& copie);

    virtual void NewGBObject();

    Element* getRoot() { return root; }
    void setRoot(Element* newRoot) {root = newRoot;}

    GBI::ObjectArray<Element>* getGBElementsByTagName(fwstring tag, int depth = -1) {return root->getGBChildrenByTagName(tag, depth);}
    vector<Element*>* getElementsByTagName(fwstring tag, int depth = -1) {return root->getChildrenByTagName(tag, depth);}
    GBI::ObjectArray<Node>* getAll();
    Element* createElement(fwstring tagName);

    virtual fwstring getContent(bool indent = false);
    void setContent(fwstring str);
    void save(string fileName);

    Element *root;
    CDocument *relob;

    static bool NoInstanciate;
    unsigned char ref;


};


#endif // DOCUMENT_H
