#ifndef GBINTERFACE_H
#define GBINTERFACE_H

#include "../gambas.h"

extern "C" GB_INTERFACE GB;

#define VARGOBJ(_type, _ob) ((_type*)VARG(_ob))
#define VPROPOBJ(_type) ((_type*)VPROP(GB_OBJECT))
#define STRINGOPT(_str, _repl, _lenrepl) MISSING(_str) ? _repl : STRING(_str),\
    MISSING(_str) ? _lenrepl : LENGTH(_str)



struct Node;
struct Attribute;

typedef struct CNode
{
    GB_BASE ob;
    Node *node;
    Attribute *curAttrEnum;
}CNode;

typedef CNode CDocument;

#endif // GBINTERFACE_H
