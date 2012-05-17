#ifndef ELEMENT_H
#define ELEMENT_H

#include "main.h"
#include "node.h"

#include "textnode.h"

#include <exception>
#include <stdexcept>

class HTMLParseException : public exception
{
public:
    HTMLParseException(unsigned int col, unsigned int line, string nnear, string err ) throw()
        : ncol(col), nline(line), error(err), near(nnear)
    {}

    virtual const char* what() const throw()
    {
        string str;
        str += "Parse error !\n" + error + "\nLine " + toString(nline) + ", column " + toString(ncol) + "\nNear : \n" + near;
        return (str).c_str();
    }

    int getCol() {return ncol;}
    void setCol(int col) {ncol = col;}
    int getLine() {return nline;}
    void setLine(int lin) {nline = lin;}
    string text() {return error;}

    ~HTMLParseException() throw() {}

private:
    int ncol;
    int nline;
    string error;
    string near;
};


vector<fwstring>* split(fwstring str, fwstring pattern);
fwstring Right(fwstring str, size_t len);
bool isLetter(const char *str);
fwstring Trim(fwstring str);
bool isLetter(fwstring &s);
bool exist(vector<fwstring> vect, fwstring elmt);
bool exist(vector<fwstring> *vect, fwstring elmt);

struct CAttrNode;

class AttrNode : public Node
{
public:
        virtual Node::Type getType() {return Node::Attribute;}
        virtual fwstring toString(int indent = -1){return textContent();}
        virtual fwstring textContent();
        virtual void setTextContent(fwstring content);
        virtual void NewGBObject();
    virtual Node* cloneNode();

    fwstring *attrName;
    fwstring *attrValue;
    void setAttrName(const fwstring &name){attrName = new fwstring(name);}
    virtual CNode* GetGBObject(){return (CNode*)relElmt;}

    CAttrNode *relElmt;

};

class AttrListElement
{
public:
    fwstring *attrName;
    fwstring *attrValue;
};

struct CElement;

class Element : public Node
{
public:
        Element();
        virtual ~Element();
        virtual Node::Type getType() {return Node::ElementNode;}
        virtual fwstring toString(int indent = -1);
        virtual fwstring textContent();
        virtual void setTextContent(fwstring content);
        virtual void setOwnerDocument(Document *doc);
        virtual Node* cloneNode();
    virtual void NewGBObject();
        virtual void SetGBObject(CElement *ob);
        virtual CNode* GetGBObject(){return (CNode*)relElmt;}

    GBI::ObjectArray<Node>* getGBChildren();
    GBI::ObjectArray<Node>* getAllChildren();
    GBI::ObjectArray<Element>* getChildrenByAttributeValue(fwstring attr, fwstring val, int depth = -1);
    GBI::ObjectArray<Element>* getGBChildrenByTagName(fwstring tag, int depth = -1);
    vector<Element*>* getChildrenByTagName(fwstring tag, int depth = -1);

    Element* getFirstChildByTagName(fwstring tag, int depth = -1);
    Element& operator=(const Element &copie);
    Node* appendChild(Node &newChild);
    Node* appendChild(Node *newChild);
    Node* prependChild(Node &newChild);
    Node* prependChild(Node *newChild);
    void ClearElements();
    void appendFromText(fwstring data, bool force = false);

    void removeChild(Node *child);
    void replaceChild(Node *oldChild, Node *newChild);

    bool insertAfter(Node *child, Node *newChild);
    bool insertBefore(Node *child, Node *newChild);

    fwstring getTagName() {return *tagName;}
    void setTagName(fwstring tag) {if(!tagName) tagName = new fwstring; *tagName = tag;}

    void appendText(fwstring text);

    fwstring getAttribute(fwstring key);
    void setAttribute(fwstring key, fwstring value);
    void addAttribute(fwstring key, fwstring value);
    bool isAttributeSet(fwstring key);

    //static vector<Node*>* fromText(fwstring data, fwstring::size_type $i = 0, uint $c = 1, uint $l = 1);
    static fvector<Node*>* fromText(fwstring data, size_t start = 0);


    GBI::ObjectArray<Element>* getChildElements();
    flist<AttrListElement*> *getAttributes() {return attributes;}

    Element* previousSibling();
    Element* nextSibling();

    Element* firstChildElement();
    Element* lastChildElement();

    bool MatchSubXPathFilter(fwstring filter);
    bool MatchXPathFilter(fwstring filter);


    flist<AttrListElement*> *attributes;
    fwstring *tagName;
    AttrNode *attributeNode;

    static vector<fwstring> singleElements;

    //Children
    size_t childCount;
    Node *firstChild;
    Node *lastChild;

    CElement *relElmt;

#ifndef HELEMENT_H

};

#endif

#endif // ELEMENT_H
