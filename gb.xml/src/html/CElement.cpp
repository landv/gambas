#include "CElement.h"
#include "element.h"

/*========== Element */

#undef THIS
#define THIS (static_cast<CElement*>(_object)->elmt)

BEGIN_PROPERTY(CElement_id)

if(READ_PROPERTY)
{
    GBI::Return((THIS->getId()).c_str());
}
else
{
    THIS->setId(PSTRING());
}

END_PROPERTY

BEGIN_PROPERTY(CElement_className)

if(READ_PROPERTY) GBI::Return((THIS->getClassName()));
else THIS->setClassName(PSTRING());

END_PROPERTY

BEGIN_METHOD(CElement_matchFilter, GB_STRING filter)

GB.ReturnBoolean(THIS->matchFilter(STRING(filter)));

END_METHOD

BEGIN_METHOD(CElement_getChildrenByFilter, GB_STRING filter; GB_INTEGER depth)

GBI::ObjectArray<Element> *array = new GBI::ObjectArray<Element>("HtmlElement", *(THIS->getChildrenByFilter(STRING(filter), VARGOPT(depth, -1))));

GB.ReturnObject(array->array);

END_METHOD

BEGIN_METHOD(CElement_getChildById, GB_STRING id; GB_INTEGER depth)

GBI::Return(THIS->getChildById(STRING(id), VARGOPT(depth, -1)));

END_METHOD

BEGIN_METHOD(CElement_getChildrenByClassName, GB_STRING className; GB_INTEGER depth)

GB.ReturnObject(THIS->getChildrenByClassName(STRING(className), VARGOPT(depth, -1))->array);

END_METHOD

GB_DESC CElementDesc[] =
{
    GB_DECLARE("XmlElement", sizeof(CElement)),


    GB_PROPERTY("Id", "s", CElement_id),
    GB_PROPERTY("ClassName", "s", CElement_className),

    GB_METHOD("MatchFilter", "b", CElement_matchFilter, "(Filter)s"),
    GB_METHOD("GetChildrenByFilter", "XmlElement[]", CElement_getChildrenByFilter, "(Filter)s[(Depth)i]"),

    GB_METHOD("GetChildById", "XmlElement", CElement_getChildById, "(Id)s[(Depth)i]"),
    GB_METHOD("GetChildrenByClassName", "XmlElement[]", CElement_getChildrenByClassName, "(ClassName)s[(Depth)i]"),



    GB_END_DECLARE
};
