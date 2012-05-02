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
    HTMLParseException(unsigned int col, unsigned int line, wstring nnear, wstring err ) throw()
        : ncol(col), nline(line), error(err), near(nnear)
    {}

    virtual const char* what() const throw()
    {
        wstring str;
        str += L"Parse error !\n" + error + L"\nLine " + toString(nline) + L", column " + toString(ncol) + L"\nNear : \n" + near;
        return WStringToString(str).c_str();
    }

    int getCol() {return ncol;}
    void setCol(int col) {ncol = col;}
    int getLine() {return nline;}
    void setLine(int lin) {nline = lin;}
    wstring text() {return error;}

    ~HTMLParseException() throw() {}

private:
    int ncol;
    int nline;
    wstring error;
    wstring near;
};


vector<wstring>* split(wstring str, wstring pattern);
wstring Right(wstring str, wstring::size_type len);
bool isLetter(const char *str);
wstring Trim(wstring str);
bool isLetter(wstring &s);
bool exist(vector<wstring> vect, wstring elmt);
bool exist(vector<wstring> *vect, wstring elmt);

class AttrNode : public Node
{
public:
    class Virtual : public Node::Virtual
    {
    public:
        Virtual(AttrNode *node) : Node::Virtual(node), parent(node) {}
        Virtual(const AttrNode::Virtual &copie) : Node::Virtual(copie.parent), parent(copie.parent) {}
        AttrNode::Virtual &operator=(const AttrNode::Virtual &copie) {parent = copie.parent; return *this;}

        virtual Node::Type getType() {return Node::Attribute;}
        virtual wstring toString(int indent = -1){return textContent();}
        virtual wstring textContent();
        virtual void setTextContent(wstring &content);

        AttrNode *parent;

    };

    wstring *attrName;
    void setAttrName(const wstring &name){attrName = new wstring(name);}

    static GB_CLASS ClassName;

};

class Element : public Node
{
public:
    class Virtual : public Node::Virtual
    {
    public:
        Virtual(Element *node) : Node::Virtual(node), parent(node) {}
        Virtual(const Element::Virtual &copie) : Node::Virtual(copie.parent), parent(copie.parent) {}
        Element::Virtual &operator=(const Element::Virtual &copie) {parent = copie.parent; return *this;}

        virtual Node::Type getType() {return Node::ElementNode;}
        virtual wstring toString(int indent = -1);
        virtual wstring textContent();
        virtual void setTextContent(wstring &content);
        virtual void setOwnerDocument(Document *doc);
        virtual Node* cloneNode();

        Element *parent;

    };
    list<Node*>* getChildren() {return children;}
    GBI::ObjectArray<Node>* getGBChildren();
    GBI::ObjectArray<Node>* getAllChildren();
    GBI::ObjectArray<Element>* getChildrenByAttributeValue(wstring attr, wstring val, int depth = -1);
    GBI::ObjectArray<Element>* getGBChildrenByTagName(wstring tag, int depth = -1);
    vector<Element*>* getChildrenByTagName(wstring tag, int depth = -1);

    Element* getFirstChildByTagName(wstring tag, int depth = -1);
    Element& operator=(const Element &copie);
    Node* appendChild(Node &newChild);
    Node* appendChild(Node *newChild);
    Node* prependChild(Node &newChild);
    Node* prependChild(Node *newChild);
    void ClearElements();
    void appendFromText(wstring data, bool force = false);

    void removeChild(Node *child);
    void replaceChild(Node *oldChild, Node *newChild);

    bool insertAfter(Node *child, Node *newChild);
    bool insertBefore(Node *child, Node *newChild);

    wstring getTagName() {return *tagName;}
    void setTagName(wstring tag) {*tagName = tag;}

    void appendText(wstring text);

    wstring getAttribute(wstring key);
    void setAttribute(wstring key, wstring value){(*attributes)[key] = value;}
    bool isAttributeSet(wstring key);

    static vector<Node*>* fromText(wstring data, wstring::size_type $i = 0, uint $c = 1, uint $l = 1);

    GBI::ObjectArray<Element>* getChildElements();
    map<wstring, wstring> *getAttributes() {return attributes;}

    Element* previousSibling();
    Element* nextSibling();

    Element* firstChildElement();
    Element* lastChildElement();

    bool MatchSubXPathFilter(wstring filter);
    bool MatchXPathFilter(wstring filter);


    map<wstring, wstring> *attributes;
    wstring *tagName;
    list<Node*> *children;
    AttrNode *attributeNode;

    static GB_CLASS ClassName;

    static vector<wstring> singleElements;

#ifndef HELEMENT_H

};


#endif

#endif // ELEMENT_H
