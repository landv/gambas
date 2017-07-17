#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "gb.xml.h"

//Switches if HTML enabled
void serializeNode(Node *node, char *&output, size_t &len, int indent = -1);
void GBserializeNode(Node *node, char *&output, size_t &len, int indent = -1);

//String output
void serializeXMLNode(Node *node, char *&output, size_t &len, int indent = -1);//Converts the node to its string representation
void GBserializeXMLNode(Node *node, char *&output, size_t &len, int indent = -1);//Same as above, but returns a Gambas String directly

//Text content
void GetXMLTextContent(Node *node, char *&output, size_t &len);
void GBGetXMLTextContent(Node *node, char *&output, size_t &len);

#endif // SERIALIZER_H
