#ifndef GB_XML_H
#define GB_XML_H

#include <stddef.h>

#define GB_STRCOMP_BINARY   0
#define GB_STRCOMP_NOCASE   1
#define GB_STRCOMP_LANG     2
#define GB_STRCOMP_LIKE     4
#define GB_STRCOMP_NATURAL  8

struct Document;
struct CNode;

typedef void* GB_ARRAY;

#if 0
#include <iostream>

using namespace std;

#define DEBUG std::cerr << "XMLDBG (" << __FILE__ << ":" <<__LINE__ << ") :"
#define DEBUGH DEBUG << endl

#endif

typedef struct Node
{
    enum Type {ElementNode, NodeText, Comment, CDATA, AttributeNode, DocumentNode, HTMLDocumentNode};

    //Node children
    Node *firstChild;
    Node *lastChild;
    size_t childCount;

    //Node tree
    Document *parentDocument;
    Node *parent;
    Node *nextNode;
    Node *previousNode;

    //Node Type
    Type type;

    //Gambas object
    CNode *GBObject;

    //User data
    void *userData; //GB_COLLECTION = void* (cf. gambas.h)
}Node;

typedef struct Attribute : public Node
{
    char *attrName;
    char *attrValue;

    size_t lenAttrName;
    size_t lenAttrValue;

}Attribute;

typedef struct Element : public Node
{
    //Tag Name
    char *tagName;
    size_t lenTagName;

    char *prefix;
    size_t lenPrefix;

    char* localName;
    size_t lenLocalName;

    Attribute *firstAttribute;
    Attribute *lastAttribute;
    size_t attributeCount;

}Element;

typedef enum {XMLDocumentType, HTMLDocumentType, XHTMLDocumentType} DocumentType ;

typedef struct Document : public Node
{
    Element *root;
    DocumentType docType;
}Document;

typedef struct TextNode : public Node
{
    char *content;
    size_t lenContent;

    char *escapedContent;
    size_t lenEscapedContent;

}TextNode;

typedef TextNode CommentNode;

typedef TextNode CDATANode;

typedef struct XMLParseException
{
    char *near;
    size_t lenNear;

    size_t line;
    size_t column;

    char *errorWhat;
}XMLParseException;

//Gambas XML component interface

#define XML_INTERFACE_VERSION 1

typedef struct
{
    void* version;
    //Converts the node to its string representation
    void (*SerializeXMLNode)(Node *node, char *&output, size_t &len, int indent);
    void (*GBSerializeXMLNode)(Node *node, char *&output, size_t &len, int indent);
    Node** (*ParseXML)(char const *data, const size_t lendata, size_t *nodeCount);
    void (*GBGetXMLTextContent)(Node *node, char *&output, size_t &len);

    //Content escaping/normalization
    void (*XMLText_escapeContent)(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
    void (*XMLText_unEscapeContent)(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);
    void (*XMLText_escapeAttributeContent)(const char *src, const size_t lenSrc, char *&dst, size_t &lenDst);

    //XMLNode Interface
    CNode* (*XMLNode_GetGBObject)(Node *node);
    Element* (*XMLNode_getFirstChildByAttributeValue)(Node *node, const char *attrName, const size_t lenAttrName,
                                                   const char *attrValue, const size_t lenAttrValue, const int mode, const int depth);
    void (*XMLNode_getGBChildrenByAttributeValue)(Node *node, const char *attrName, const size_t lenAttrName,
                                       const char *attrValue, const size_t lenAttrValue,
                                       GB_ARRAY *array, const int mode, const int depth);

    Element* (*XMLNode_firstChildElement)(Node *node);
    Element* (*XMLNode_lastChildElement)(Node *node);
    Element* (*XMLNode_previousElement)(const Node *node);
    void (*XMLNode_appendChild)(Node *node, Node *newChild);
    Element** (*XMLNode_getChildrenByTagName)(Node *node, const char *ctagName, const size_t clenTagName, size_t &lenArray, const int depth);
    Element* (*XMLNode_getFirstChildByTagName)(const Node *node, const char *ctagName, const size_t clenTagName, const int depth);

    void (*XMLNode_setTextContent)(Node *node, const char *content, const size_t lenContent);


    //XMLTextNode Interface
    TextNode* (*XMLTextNode_New)(const char *ncontent, const size_t nlen);
    void (*XMLTextNode_setEscapedTextContent)(TextNode *node, const char *ncontent, const size_t nlen);

    void (*XMLTextNode_checkEscapedContent)(TextNode *node);

    //XMLDocument Interface
    Document* (*XMLDocument_New)();
    Document* (*XMLDocument_NewFromFile)(const char *fileName, const size_t lenFileName, const DocumentType docType);
    void (*XMLDocument_SetContent)(Document *doc, const char *content, size_t len);

    //XMLElement Interface
    Element* (*XMLElement_New)(const char *ntagName, size_t nlenTagName);
    void (*XMLElement_SetTagName)(Element *elmt, const char *ntagName, size_t nlenTagName);
    bool (*XMLElement_AttributeContains)(const Element *elmt, const char *attrName, size_t lenAttrName, const char *value, size_t lenValue);

    Attribute* (*XMLElement_AddAttribute)(Element *elmt, const char *ntagName, size_t nlenTagName,
                                    const char *nattrVal, const size_t nlenAttrVal);
    Attribute* (*XMLElement_GetAttribute)(const Element *elmt, const char *nattrName, const size_t nlenAttrName, const int mode);
    void (*XMLElement_SetAttribute)(Element *elmt, const char *nattrName, const size_t nlenAttrName,
                                 const char *nattrVal, const size_t nlenAttrVal);

    void (*XMLElement_RemoveAttribute)(Element *elmt, Attribute *attr);
    //XMLComment Interface
    CommentNode* (*XMLComment_New)(const char *ncontent, const size_t nlen);

    //XMLCDATA Interface
    CDATANode* (*XMLCDATA_New)(const char *ncontent, const size_t nlen);



    void (*ReturnNode)(Node *node);



    //Various utils
    void (*Trim)(const char* &str, size_t &len);
    bool (*isNameStartChar)(const wchar_t car);
    bool (*isNameChar)(const wchar_t car);
    const void* (*memchrs)(const char *source, size_t lensource, const char *comp, size_t lencomp);
    bool (*GB_MatchString)(const char *str, size_t lenStr, const char *pattern, size_t lenPattern, int mode);
    wchar_t (*nextUTF8Char)(const char *&data, size_t len);
    bool (*isWhiteSpace)(const wchar_t s);

    void (*ThrowXMLParseException)(const char* nerror, const char *text, const size_t lenText, const char *posFailed);

#if defined(OS_MACOSX) || defined(__APPLE__)
    void* (*memrchr)(const char *s, int c, size_t n);
#endif

    void *_null;

}XML_INTERFACE;


#endif // GB_XML_H
