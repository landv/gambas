#ifndef HELEMENT_H
#define HELEMENT_H

#include "main.h"
#include "../element.h"


//class Element:
//{

    wstring getClassName(){return getAttribute(L"class");}
    vector<wstring>* getClassNames();
    void setClassName(wstring value){setAttribute(L"class", value);}

    wstring getId(){return getAttribute(L"id");}
    void setId(wstring value){setAttribute(L"id", value);}

    bool matchSubFilter(wstring filter);
    bool matchFilter(wstring filter);
    vector<Element*>* getChildrenByFilter(wstring filter, int depth = -1);

    Element* getChildById(wstring id, int depth = -1)
    {
        GBI::ObjectArray<Element>* elmts = getChildrenByAttributeValue(L"id", id, depth);
        return elmts->size() > 0 ? elmts->at(0) : 0;
    }
    GBI::ObjectArray<Element>* getChildrenByClassName(wstring className, int depth = -1){return getChildrenByAttributeValue(L"class", className, depth);}


};


#endif
