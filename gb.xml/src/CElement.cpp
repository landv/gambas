#include "CElement.h"
#include "element.h"

/*========== Element */

#undef THIS
#define THIS (static_cast<CElement*>(_object)->elmt)


BEGIN_METHOD(CElement_new, GB_STRING tagName)

if(Node::NoInstanciate) return;
THIS = new Element;
static_cast<CNode*>(_object)->node = THIS;
THIS->SetGBObject(static_cast<CElement*>(_object));
if(!MISSING(tagName)) THIS->setTagName(STRING(tagName));
//if(!MISSING(doc)) THIS->setOwnerDocument(reinterpret_cast<Document*>(VARG(doc)));


END_METHOD

BEGIN_METHOD_VOID(CElement_free)



END_METHOD

BEGIN_PROPERTY(CElement_tagName)

if(READ_PROPERTY)
{
    //DEBUG << THIS->tagName->toStdString() << endl;
    GBI::Return((THIS->getTagName()));
}
else
{
    THIS->setTagName(PSTRING());
}

END_PROPERTY

BEGIN_METHOD(CElement_AppendChild, GB_OBJECT newChild)

//DEBUG << VARGOBJ(CNode,newChild)->node << endl;
THIS->appendChild(VARGOBJ(CNode,newChild)->node);

END_METHOD

BEGIN_METHOD(CElement_AppendChildren, GB_OBJECT children)

GB_ARRAY array = reinterpret_cast<GB_ARRAY>(VARG(children));

for(int i = 0; i < GB.Array.Count(array); i++)
{
    THIS->appendChild(*(reinterpret_cast<Node**>(GB.Array.Get(array, i))));
}

END_METHOD

BEGIN_METHOD(CElement_AppendText, GB_STRING text)

DEBUG << STRING(text).toStdString() << endl;
THIS->appendText(STRING(text));

END_METHOD

BEGIN_PROPERTY(CElement_childNodes)

GB.ReturnObject(THIS->getGBChildren()->array);

END_PROPERTY

BEGIN_PROPERTY(CElement_childElements)

GB.ReturnObject(THIS->getChildElements()->array);

END_PROPERTY

BEGIN_METHOD(CElement_getChildrenByTagName, GB_STRING tagName; GB_INTEGER depth)

GB.ReturnObject(THIS->getGBChildrenByTagName(STRING(tagName), VARGOPT(depth, -1))->array);

END_METHOD

BEGIN_PROPERTY(CElement_allChildNodes)

GB.ReturnObject(THIS->getAllChildren()->array);

END_PROPERTY

BEGIN_METHOD(CElement_fromText, GB_STRING data)

GBI::ObjectArray<Node> *array = new GBI::ObjectArray<Node>("XmlNode", *(Element::fromText(STRING(data))));
GB.ReturnObject(array->array);

END_METHOD

BEGIN_METHOD(CElement_appendFromText, GB_STRING data)

try{
THIS->appendFromText(STRING(data));
}
catch(HTMLParseException &e)
{
    GB.Error(e.what());
}

END_METHOD

BEGIN_METHOD(CElement_getChildrenByAttributeValue, GB_STRING attr; GB_STRING val; GB_INTEGER depth)

GB.ReturnObject(THIS->getChildrenByAttributeValue(STRING(attr), STRING(val), VARGOPT(depth, -1))->array);

END_METHOD

BEGIN_METHOD(CElement_removeChild, GB_OBJECT oldChild)

THIS->removeChild(reinterpret_cast<Node*>(VARG(oldChild)));

END_METHOD

BEGIN_METHOD(CElement_replaceChild, GB_OBJECT oldChild; GB_OBJECT newChild)

THIS->replaceChild(reinterpret_cast<Node*>(VARG(oldChild)),
                        reinterpret_cast<Node*>(VARG(newChild)));

END_METHOD

BEGIN_METHOD(CElement_getAttribute, GB_STRING attr)

GBI::Return((THIS->getAttribute(STRING(attr))));

END_METHOD

BEGIN_METHOD(CElement_setAttribute, GB_STRING attr; GB_STRING val)

THIS->setAttribute(STRING(attr), STRING(val));

END_METHOD

BEGIN_PROPERTY(CElement_previousSibling)

GBI::Return(THIS->previousSibling());

END_PROPERTY

BEGIN_PROPERTY(CElement_nextSibling)

GBI::Return(THIS->nextSibling());

END_PROPERTY

BEGIN_METHOD(CElement_isAttributeSet, GB_STRING attr)

GB.ReturnBoolean(THIS->isAttributeSet(STRING(attr)));

END_METHOD

BEGIN_PROPERTY(CElement_firstChildElement)

GBI::Return(THIS->firstChildElement());

END_PROPERTY

BEGIN_PROPERTY(CElement_lastChildElement)

GBI::Return(THIS->lastChildElement());

END_PROPERTY

BEGIN_METHOD(CElement_prependChild, GB_OBJECT newChild)

THIS->prependChild(VARGOBJ(Node, newChild));

END_METHOD

BEGIN_METHOD(CElement_insertAfter, GB_OBJECT child; GB_OBJECT newChild)

THIS->insertAfter(VARGOBJ(Node, child), VARGOBJ(Node, newChild));

END_METHOD

BEGIN_METHOD(CElement_insertBefore, GB_OBJECT child; GB_OBJECT newChild)

THIS->insertBefore(VARGOBJ(Node, child), VARGOBJ(Node, newChild));

END_METHOD

BEGIN_METHOD(CElement_newElement, GB_STRING name; GB_STRING value)

Element *elmt = new Element;
elmt->setTagName(STRING(name));
if(!MISSING(value)) elmt->setTextContent(STRING(value));
THIS->appendChild(elmt);

END_METHOD

BEGIN_METHOD(CElement_matchXPathFilter, GB_STRING filter)

GB.ReturnBoolean(THIS->MatchXPathFilter(STRING(filter)));

END_METHOD


GB_DESC CElementDesc[] =
{
    GB_DECLARE("XmlElement", sizeof(CElement)), GB_INHERITS("XmlNode"),
    GB_METHOD("_new", "", CElement_new, "[(TagName)s]"),
    GB_METHOD("_free", "", CElement_free, ""),
    GB_METHOD("AppendChild", "", CElement_AppendChild, "(NewChild)XmlNode"),
    GB_METHOD("AppendChildren", "", CElement_AppendChildren, "(NewChildren)XmlNode[]"),
    GB_METHOD("PrependChild", "", CElement_prependChild, "(NewChild)XmlNode"),
    GB_METHOD("InsertAfter", "", CElement_insertAfter, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("InsertBefore", "", CElement_insertBefore, "(Child)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("AppendText", "", CElement_AppendText, "(NewText)s"),
    GB_METHOD("AppendFromText", "", CElement_appendFromText, "(Data)s"),
    GB_METHOD("RemoveChild", "", CElement_removeChild, "(OldChild)XmlNode"),
    GB_METHOD("ReplaceChild", "", CElement_replaceChild, "(OldChild)XmlNode;(NewChild)XmlNode"),
    GB_METHOD("NewElement", "", CElement_newElement, "(Name)s[(Value)s]"),

    GB_PROPERTY("TagName", "s", CElement_tagName),

    GB_PROPERTY_READ("PreviousSibling", "XmlElement", CElement_previousSibling),
    GB_PROPERTY_READ("NextSibling", "XmlElement", CElement_nextSibling),

    GB_METHOD("GetAttribute", "s", CElement_getAttribute, "(Name)s"),
    GB_METHOD("SetAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    GB_METHOD("NewAttribute", "", CElement_setAttribute, "(Name)s(Value)s"),
    GB_METHOD("IsAttributeSet", "b", CElement_isAttributeSet, "(Name)s"),

    GB_PROPERTY_READ("ChildNodes", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("Children", "XmlNode[]", CElement_childNodes),
    GB_PROPERTY_READ("ChildElements", "XmlElement[]", CElement_childElements),
    GB_PROPERTY_READ("AllChildNodes", "XmlNode[]", CElement_allChildNodes),
    GB_PROPERTY_READ("FirstChildElement", "XmlElement", CElement_firstChildElement),
    GB_PROPERTY_READ("LastChildElement", "XmlElement", CElement_lastChildElement),

    GB_METHOD("MatchXPathFilter", "b", CElement_matchXPathFilter ,"(Filter)s"),
    GB_METHOD("GetChildrenByTagName", "XmlElement[]", CElement_getChildrenByTagName, "(TagName)s[(Depth)i]"),
    GB_METHOD("GetChildrenByAttributeValue", "XmlElement[]", CElement_getChildrenByAttributeValue, "(Attribute)s(Value)s[(Depth)i]"),

    GB_STATIC_METHOD("FromText", "XmlNode[]", CElement_fromText, "(Data)s"),

    GB_END_DECLARE
};


