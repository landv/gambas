#ifndef GB_XML_HTML_H
#define GB_XML_HTML_H

#include "../gb.xml.h"

#define XML_HTML_INTERFACE_VERSION 1

typedef struct
{
    int version;
    //Converts the node to its string representation
    void (*serializeHTMLNode)(Node *node, char *&output, size_t &len, int indent);
    void (*GBserializeHTMLNode)(Node *node, char *&output, size_t &len, int indent);

    //Parser
    Node** (*parseHTML)(char const *data, const size_t lendata, size_t *nodeCount);
    void (*GBparseHTML)(char const *data, const size_t lendata, GB_ARRAY *array);

    //HtmlDocument Interface
    Document* (*HtmlDocument_New)();
    Document* (*HtmlDocument_NewFromFile)(const char *fileName, const size_t lenFileName);
    void *_null;

}XML_HTML_INTERFACE;

#endif // GB_XML_HTML_H
