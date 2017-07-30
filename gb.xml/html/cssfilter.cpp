#include "cssfilter.h"
#include "htmlmain.h"
#include "htmlelement.h"
#include <memory.h>

bool HTMLElement_MatchSubFilter(const Element *elmt, const char *filter, size_t lenFilter);

bool HTMLElement_MatchFilter(const Element *elmt, const char *filter, size_t lenFilter)
{
    if(!lenFilter) return true;
    XML.Trim(filter, lenFilter);
    char *pos;

    pos = (char*)memrchr(filter, ',', lenFilter);
    if(pos)
    {
        return HTMLElement_MatchFilter(elmt, filter, (pos - filter)) ||
                HTMLElement_MatchFilter(elmt, pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, '>', lenFilter);
    if(pos)
    {
        Element *parent = (Element*)(elmt->parent);
        if(!parent) return false;
        return HTMLElement_MatchFilter(parent, filter, (pos - filter))  &&
                HTMLElement_MatchFilter(elmt, pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, '+', lenFilter);
    if(pos)
    {
        Element *previous = XML.XMLNode_previousElement(elmt);
        if(!previous) return false;
        return HTMLElement_MatchFilter(previous, filter, (pos - filter))  &&
                HTMLElement_MatchFilter(previous, pos, lenFilter - (pos + 1 - filter));
    }

    pos = (char*)memrchr(filter, ' ', lenFilter);
    if(pos)
    {
        if(!HTMLElement_MatchFilter(elmt, pos, lenFilter - (pos + 1 - filter))) return false;
        for(Node *parent = elmt->parent; parent != 0; parent = parent->parent)//TODO: does not support non-element parents
        {
            if(parent->type == Node::ElementNode)
            {
                if(HTMLElement_MatchFilter((Element*)parent, filter, (pos - filter))) return true;
            }
        }

        return false;
    }

    return HTMLElement_MatchSubFilter(elmt, filter, lenFilter);
}

bool HTMLElement_MatchSubFilter(const Element *elmt, const char *filter, size_t lenFilter)
{
    if(!lenFilter) return true;
    XML.Trim(filter, lenFilter);
    if(!lenFilter) return true;

    char s = 0;
    char const *pos = 0;

    for(pos = filter + 1; pos < filter + lenFilter; ++pos)
    {
        if(!XML.isNameChar(*pos))//Something else that a name
        {
            break;
        }
    }

    bool cond = (pos != filter + lenFilter);//If there is something else to check

    s = *filter;
    if(s == '*')//Universal selector
    {
        if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
        return true;
    }
    if(s == ':')//Pseudo-class
    {
        register size_t lenSubStr = pos - filter;

        if(lenSubStr == 11 && !memcmp(filter, "first-child", 11))
        {
            if(!elmt->parent) return false;
            if(XML.XMLNode_firstChildElement(elmt->parent) != elmt) return false;
            if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
            return true;
        }
        else if(lenSubStr == 10 && !memcmp(filter, "last-child", 10))
        {
            if(!elmt->parent) return false;
            if(XML.XMLNode_lastChildElement(elmt->parent) != elmt) return false;
            if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
            return true;
        }
        return false;
    }
    if(XML.isNameStartChar(s))//Tag Name
    {
        if(!(elmt->lenTagName + filter == pos)) return false;//lenTagName == pos - filter
        if(memcmp(elmt->tagName, filter, elmt->lenTagName)) return false;
        if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
        return true;
    }
    else if(s == '#')//ID
    {
        Attribute *id = HTMLElement_GetId(elmt);
        if(!id) return false;

        if(!(id->lenAttrValue + filter + 1 == pos)) return false;
        if(memcmp(filter + 1, id->attrValue, pos - (filter + 1))) return false;
        if(cond) return (HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter)));
        return true;
    }
    else if(s == '.')//ClassName
    {
        if(!HTMLElement_HasClassName(elmt, filter + 1, pos - (filter + 1))) return false;
        if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - (filter + 1)));
        return true;
    }
    else if(s == '[')//Attribut
    {
        //Syntax : [foo="bar"]
        char const *endPos = (char*)memchr(filter, ']', lenFilter);//On cherche le crochet fermant

        endPos = endPos ? endPos : filter + lenFilter - 1;
        pos = (endPos + 1);
        cond = (pos < filter + lenFilter);

        char *equalPos = (char*)memchr(filter, '=', lenFilter);//On cherche le signe égal

        if(equalPos)//Si trouvé
        {
            s = *(equalPos - 1);//Le signe avant le signe égal
            char const *attrName = filter + 1; size_t lenAttrName = (equalPos - filter - 1);
            char const *attrValue = equalPos + 2; size_t lenAttrValue = (endPos - equalPos - 3);
            if(s == '~')// ~= Comparison
            {
                if(!XML.XMLElement_AttributeContains(elmt, attrName, lenAttrName - 1, attrValue, lenAttrValue)) return false;
                if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == '^')// ^= Comparison
            {
                Attribute *attr = XML.XMLElement_GetAttribute(elmt, attrName, lenAttrName - 1, 0);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(memcmp(attr->attrValue, attrValue, lenAttrValue)) return false;
                if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == '$')// $= Comparison
            {
                Attribute *attr = XML.XMLElement_GetAttribute(elmt, attrName, lenAttrName - 1, 0);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(memcmp(attr->attrValue + attr->lenAttrValue - lenAttrValue, attrValue, lenAttrValue)) return false;
                if(cond) HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
                return true;
            }
            if(s == '*')// *= Comparison
            {
                Attribute *attr = XML.XMLElement_GetAttribute(elmt, attrName, lenAttrName - 1, 0);
                if(!attr) return false;
                if(attr->lenAttrValue < lenAttrValue) return false;
                if(!XML.memchrs(attr->attrValue, attr->lenAttrValue, attrValue, lenAttrValue)) return false;
                if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
                return true;
            }

            Attribute *attr = XML.XMLElement_GetAttribute(elmt, attrName, lenAttrName, 0);
            if(!attr) return false;
            if(attr->lenAttrValue != lenAttrValue) return false;
            if(memcmp(attr->attrValue, attrValue, lenAttrValue)) return false;

            //Valeur de l'attribut
            if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
            return true;
        }

        Attribute *attr = XML.XMLElement_GetAttribute(elmt, filter + 1, endPos - filter - 1, 0);
        if(!attr) return false;
        //Si l'attribut est défini
        if(cond) return HTMLElement_MatchSubFilter(elmt, pos, lenFilter - (pos - filter));
        return true;

    }

    return false;

}
