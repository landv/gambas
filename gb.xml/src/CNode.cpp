#include "node.h"

/*========== Node */

#undef THIS
#define THIS (static_cast<Node*>(_object))

BEGIN_METHOD(CNode_toString, GB_BOOLEAN indent)

    GB.ReturnNewZeroString(WStringToString(THIS->toString((!MISSING(indent) && VARG(indent)) ? 0 : -1)).c_str());

END_METHOD

BEGIN_PROPERTY(CNode_textContent)

if(READ_PROPERTY)
{
    GB.ReturnNewZeroString(WStringToString(THIS->textContent()).c_str());
}
else
{
THIS->setTextContent(PSTRING());
}

END_PROPERTY

#define NODE_BASE 0
#define NODE_ELEMENT 1
#define NODE_TEXT 2
#define NODE_COMMENT 3
#define NODE_CDATA 4
#define NODE_ATTRIBUTE 5

BEGIN_PROPERTY(CNode_type)

switch(THIS->getType())
{
    case Node::ElementNode:
        GB.ReturnInteger(NODE_ELEMENT);break;
    case Node::Comment:
        GB.ReturnInteger(NODE_COMMENT);break;
    case Node::NodeText:
        GB.ReturnInteger(NODE_TEXT);break;
    case Node::CDATA:
        GB.ReturnInteger(NODE_CDATA);break;
    case Node::BaseNode:
    default:
        GB.ReturnInteger(NODE_BASE);
}

END_PROPERTY

BEGIN_PROPERTY(CNode_isElement)

GB.ReturnBoolean(THIS->isElement());

END_PROPERTY

BEGIN_PROPERTY(CNode_isText)

GB.ReturnBoolean(THIS->isText());

END_PROPERTY

BEGIN_PROPERTY(CNode_isComment)

GB.ReturnBoolean(THIS->isComment());

END_PROPERTY

BEGIN_PROPERTY(CNode_isCDATA)

GB.ReturnBoolean(THIS->isCDATA());

END_PROPERTY

BEGIN_PROPERTY(CNode_element)

GB.ReturnObject(THIS->toElement());

END_PROPERTY

BEGIN_PROPERTY(CNode_ownerDoc)

GB.ReturnObject(THIS->ownerDocument());

END_PROPERTY

BEGIN_PROPERTY(CNode_parent)

GB.ReturnObject(THIS->getParent());

END_PROPERTY

BEGIN_PROPERTY(CNode_previous)

if(!READ_PROPERTY) return;
GB.ReturnObject(THIS->previous());

END_PROPERTY

BEGIN_PROPERTY(CNode_next)

if(!READ_PROPERTY) return;
GB.ReturnObject(THIS->next());

END_PROPERTY

BEGIN_PROPERTY(CNode_name)

if(!READ_PROPERTY)
{
    if(THIS->isElement())
    {
        THIS->toElement()->setTagName(PSTRING());
    }
    return;
}

switch (THIS->getType())
{
    case Node::ElementNode:
        GB.ReturnNewZeroString(WStringToString(*(THIS->toElement()->tagName)).c_str());break;
    case Node::NodeText:
        GB.ReturnNewZeroString("#text");break;
    case Node::Comment:
        GB.ReturnNewZeroString("#comment");break;
    case Node::CDATA:
        GB.ReturnNewZeroString("#cdata");break;
    case Node::Attribute:
        GB.ReturnNewZeroString(WStringToString(*(reinterpret_cast<AttrNode*>(THIS)->attrName)).c_str()); break;
    case Node::BaseNode:
    default:
        GB.ReturnNull();
}

END_PROPERTY

BEGIN_METHOD(CNode_newElement, GB_STRING name; GB_STRING value)

if(!THIS->isElement()) return;
Element *elmt = GBI::New<Element>();
elmt->setTagName(STRING(name));
if(!MISSING(value)) elmt->setTextContent(STRING(value));
THIS->toElement()->appendChild(elmt);

END_METHOD

BEGIN_METHOD(CNode_setAttribute, GB_STRING attr; GB_STRING val)

if(!THIS->isElement()) return;
THIS->toElement()->setAttribute(STRING(attr), STRING(val));

END_METHOD


BEGIN_METHOD(CElementAttributes_get, GB_STRING name)

GB.ReturnNewZeroString(WStringToString(THIS->toElement()->getAttribute(STRING(name))).c_str());

END_METHOD

BEGIN_METHOD(CElementAttributes_put, GB_STRING value; GB_STRING name)

THIS->toElement()->setAttribute(STRING(name), STRING(value));

END_METHOD

BEGIN_PROPERTY(CElementAttributes_count)

if(READ_PROPERTY)
{
    GB.ReturnInteger(THIS->toElement()->attributes->size());
}

END_PROPERTY

BEGIN_METHOD_VOID(CElementAttributes_next)

map<wstring,wstring>::iterator *it = *reinterpret_cast<map<wstring,wstring>::iterator**>((GB.GetEnum()));
if(it == 0)
{
    it = new map<wstring,wstring>::iterator(THIS->toElement()->attributes->begin());
    *reinterpret_cast<map<wstring,wstring>::iterator**>(GB.GetEnum()) = it;
}
else
{
    ++(*it);
}

if(*it == THIS->toElement()->attributes->end()) {GB.StopEnum(); delete it; THIS->toElement()->attributeNode->attrName = 0; return;}

THIS->toElement()->attributeNode->setAttrName(((*(*it)).first));
GB.ReturnObject(THIS->toElement()->attributeNode);


END_METHOD


BEGIN_PROPERTY(CNode_attributes)

if(THIS->getType() == NODE_ELEMENT)
{
    RETURN_SELF();
    return;
}

GB.ReturnNull();

END_PROPERTY

GB_DESC CElementAttributesDesc[] =
{
    GB_DECLARE(".XmlElementAttributes", 0), GB_VIRTUAL_CLASS(),
    GB_METHOD("_get", "s", CElementAttributes_get, "(Name)s"),
    GB_METHOD("_put", "s", CElementAttributes_put, "(Value)s(Name)s"),
    GB_METHOD("_next", "XmlNode", CElementAttributes_next, ""),
    GB_PROPERTY_READ("Count", "i", CElementAttributes_count),
    GB_END_DECLARE
};

GB_DESC CElementAttributeNodeDesc[] =
{
    GB_DECLARE("_XmlAttrNode", sizeof(AttrNode)), GB_INHERITS("XmlNode"),

    GB_END_DECLARE
};

GB_DESC CNodeDesc[] =
{
    GB_DECLARE("XmlNode", sizeof(Node)), GB_NOT_CREATABLE(),

    GB_CONSTANT("ElementNode", "i", NODE_ELEMENT),
    GB_CONSTANT("TextNode", "i", NODE_TEXT),
    GB_CONSTANT("CommentNode", "i", NODE_COMMENT),
    GB_CONSTANT("CDATASectionNode", "i", NODE_CDATA),
    GB_CONSTANT("AttributeNode", "i", NODE_ATTRIBUTE),

    GB_PROPERTY_READ("Type", "i", CNode_type),
    GB_PROPERTY_READ("IsElement", "b", CNode_isElement),
    GB_PROPERTY_READ("IsText", "b", CNode_isText),
    GB_PROPERTY_READ("IsComment", "b", CNode_isComment),
    GB_PROPERTY_READ("IsCDATA", "b", CNode_isCDATA),

    GB_PROPERTY_READ("Element", "XmlElement", CNode_element),
    GB_PROPERTY_READ("OwnerDocument", "XmlDocument", CNode_ownerDoc),
    GB_PROPERTY_READ("Parent", "XmlElement", CNode_parent),
    GB_PROPERTY_READ("Previous", "XmlNode", CNode_previous),
    GB_PROPERTY_READ("Next", "XmlNode", CNode_next),

    GB_METHOD("ToString", "s", CNode_toString, "[(Indent)b]"),
    GB_PROPERTY("TextContent", "s", CNode_textContent),
    GB_PROPERTY("Value", "s", CNode_textContent),
    GB_PROPERTY("Name", "s", CNode_name),
    
    //Méthodes obsolètes
    GB_METHOD("NewElement", "s", CNode_newElement, "(Name)s[(Value)s]"),
    GB_METHOD("NewAttribute", "", CNode_setAttribute, "(Name)s(Value)s"),
    GB_PROPERTY_READ("Attributes", ".XmlElementAttributes", CNode_attributes),
    
    //Constantes obsolètes
      GB_CONSTANT("EntityRefNode", "i", 0),
      GB_CONSTANT("EntityNode", "i", 0),
      GB_CONSTANT("PiNode", "i", 0),
      GB_CONSTANT("DocumentNode", "i", 0),
      GB_CONSTANT("DocumentTypeNode", "i", 0),
      GB_CONSTANT("DocumentFragNode", "i", 0),
      GB_CONSTANT("NotationNode", "i", 0),
      GB_CONSTANT("HtmlDocumentNode", "i", 0),
      GB_CONSTANT("DtdNode", "i", 0),
      GB_CONSTANT("ElementDecl", "i", 0),
      GB_CONSTANT("AttributeDecl", "i", 0),
      GB_CONSTANT("EntityDecl", "i", 0),
      GB_CONSTANT("NamespaceDecl", "i", 0),
      GB_CONSTANT("XIncludeStart", "i", 0),
      GB_CONSTANT("XIncludeEnd", "i", 0),
      GB_CONSTANT("DocbDocumentNode", "i", 0),

    GB_END_DECLARE
};


