#ifndef HELEMENT_H
#define HELEMENT_H

#include "main.h"
#include "../element.h"


//class Element:
//{

    fwstring getClassName(){return getAttribute("class");}
    vector<fwstring>* getClassNames();
    void setClassName(fwstring value){setAttribute("class", value);}

    fwstring getId(){return getAttribute("id");}
    void setId(fwstring value){setAttribute("id", value);}

    bool matchSubFilter(fwstring filter);
    bool matchFilter(fwstring filter);
    vector<Element*>* getChildrenByFilter(fwstring filter, int depth = -1);

    Element* getChildById(fwstring id, int depth = -1)
    {
        GBI::ObjectArray<Element>* elmts = getChildrenByAttributeValue("id", id, depth);
        return elmts->size() > 0 ? elmts->at(0) : 0;
    }
    GBI::ObjectArray<Element>* getChildrenByClassName(fwstring className, int depth = -1){return getChildrenByAttributeValue("class", className, depth);}


};


#endif
