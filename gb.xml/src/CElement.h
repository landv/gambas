#ifndef CELEMENT_H
#define CELEMENT_H

#include "CNode.h"

class Element;

typedef struct CElement
{
    CNode n;
    Element *elmt;
}CElement;

#ifndef CLASSES_CPP
extern GB_DESC CElementDesc[];
#endif

#endif // CELEMENT_H
