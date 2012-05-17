#ifndef CNODE_H
#define CNODE_H

#include "../gambas.h"
#include "../gb_common.h"

class Node;

typedef struct CNode
{
    GB_BASE ob;
    Node *node;
} CNode;

#ifndef CLASSES_CPP
extern GB_DESC CNodeDesc[];
extern GB_DESC CElementAttributesDesc[];
extern GB_DESC CElementAttributeNodeDesc[];
#endif

#endif // CNODE_H
