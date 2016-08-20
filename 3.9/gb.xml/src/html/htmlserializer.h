#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "htmlmain.h"

void serializeHTMLNode(Node *node, char *&output, size_t &len, int indent = -1);//Converts the node to its string representation
void GBserializeHTMLNode(Node *node, char *&output, size_t &len, int indent = -1);//Same as above, but returns a Gambas String directly


#endif // SERIALIZER_H
