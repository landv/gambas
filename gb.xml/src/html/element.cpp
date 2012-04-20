#include "element.h"

#include "../element.cpp"
#include "../node.cpp"
#include "../textnode.cpp"

vector<wstring>* Element::getClassNames()
{
    return split(this->getClassName(), L" ");
}

bool Element::matchSubFilter(wstring filt)
{
    if(!filt.length()) return false;
    wstring filter = Trim(filt);
    wstring s;
    unsigned int pos = 0;
    for(unsigned int i = 1; i < filter.size(); i++)
    {
        s = filter.at(i);
        if(!isLetter(s) && !(s == L"-"))//Autre chose qu'un nom
        {
            pos = i;
            break;
        }
    }

    bool cond = pos;//Si il y a autre chose à évaluer

    s = filter.at(0);
    if(s == L"*")//Sélecteur universel
    {
        if(cond) return matchSubFilter(filter.substr(pos));
        return true;
    }
    if(s == L":")//Pseudo-classe
    {
        wstring substr;
        if(cond) substr = filter.substr(1, pos - 1);
        else substr = filter.substr(1);
        if(substr == L"first-child")
        {
            if(!parent) return false;
            if(cond) return this->parent->firstChildElement() == this && matchSubFilter(filter.substr(pos));
            return this->parent->firstChildElement() == this;
        }
        if(substr == L"last-child")
        {
            if(!parent) return false;
            if(cond) return this->parent->lastChildElement() == this && matchSubFilter(filter.substr(pos));
            return this->parent->lastChildElement() == this;
        }
        return false;
    }
    if(isLetter(s))//C'est un nom de tag (rien au début)
    {
        if(cond) return (this->getTagName() == filter.substr(0, pos) && matchSubFilter(filter.substr(pos)));
        return (this->getTagName() == filter);
    }
    else if(s == L"#")//C'est un id
    {
        if(cond) return (this->getId() == filter.substr(1, pos - 1) && matchSubFilter(filter.substr(pos)));
        return (this->getId() == filter.substr(1));
    }
    else if(s == L".")//ClassName
    {
        if(cond) return (exist(getClassNames(),filter.substr(1, pos - 1)) && matchSubFilter(filter.substr(pos)));
        return (exist(getClassNames(),filter.substr(1)));
    }
    else if(s == L"[")//Attribut
    {
        wstring::size_type i;
        for(i = 1; i < filter.length(); i++)//On cherche le crochet fermant
        {
            s = filter.at(i);
            if(s == L"]") break;
        }

        wstring::size_type j = i;
        cond = (j < (filter.length()-1));

        wstring content = filter.substr(1, i - 1);
        for(i = 1; i < content.length(); i++)//On cherche le signe égal
        {
            s = content.at(i);
            if(s == L"=") break;
        }

        if(i != content.length())//Si trouvé
        {
            s = content.at(i - 1);
            wstring attrName = content.substr(0, i);
            wstring attrNameSgn = content.substr(0, i - 1);
            wstring attrValue = content.substr(i+2, content.length() - (i + 3));
            if(s == L"~")//Comparaison ~=
            {
                if(cond) return exist(split(getAttribute(attrNameSgn), L" "), attrValue) && matchSubFilter(filter.substr(j + 1));
                return exist(split(getAttribute(attrNameSgn), L" "), attrValue);
            }
            if(s == L"^")
            {
                if(cond) return getAttribute(attrNameSgn).substr(0, attrValue.length()) == attrValue && matchSubFilter(filter.substr(j + 1));
                return getAttribute(attrNameSgn).substr(0, attrValue.length()) == attrValue;
            }
            if(s == L"$")
            {
                if(cond) return Right(getAttribute(attrNameSgn), attrValue.length()) == attrValue && matchSubFilter(filter.substr(j + 1));
                return Right(getAttribute(attrNameSgn), attrValue.length()) == attrValue;
            }
            if(s == L"*")
            {
                if(cond) return (getAttribute(attrNameSgn).rfind(attrValue) != wstring::npos) && matchSubFilter(filter.substr(j + 1));
                return getAttribute(attrNameSgn).rfind(attrValue) != wstring::npos;
            }

            //Valeur de l'attribut
            if(cond) return getAttribute(attrName) == attrValue && matchSubFilter(filter.substr(j + 1));
            return getAttribute(attrName) == attrValue;
        }

        //Si l'attribut est défini
        if(cond) return isAttributeSet(content) && matchSubFilter(filter.substr(j + 1));
        return isAttributeSet(content);

    }

    return false;
}

bool Element::matchFilter(wstring filt)
{
    if(!filt.length()) return false;
    wstring filter = Trim(filt);
    wstring::size_type pos;

    pos = filter.rfind(L",");
    if(pos != wstring::npos)
    {
        return (matchFilter(filter.substr(0, pos)) || matchFilter(filter.substr(pos + 1)));
    }

    pos = filter.rfind(L">");
    if(pos != wstring::npos)
    {
        Element *elmt = this->parent;
        if(!elmt) return false;
        return (elmt->matchFilter(filter.substr(0, pos)) && matchFilter(filter.substr(pos + 1)));
    }

    pos = filter.rfind(L"+");
    if(pos != wstring::npos)
    {
        Element *elmt = this->previousSibling();
        if(!elmt) return false;
        return (elmt->matchFilter(filter.substr(0, pos)) && matchFilter(filter.substr(pos + 1)));
    }

    pos = filter.rfind(L" ");
    if(pos != wstring::npos)
    {
        if(!matchFilter(filter.substr(pos + 1))) return false;
        Element *elmt = this->parent;
        while(elmt)
        {
            if(elmt->matchFilter(filter.substr(0, pos))) return true;
            elmt = elmt->parent;
        }

        return false;
    }

    return matchSubFilter(filter);


}

vector<Element*>* Element::getChildrenByFilter(wstring filter, int depth)
{
    vector<Element*> *elements = new vector<Element*>;
    if(depth == 0) return elements;
    if(matchFilter(filter)) elements->push_back(this);
    if(depth == 1) return elements;
    vector<Element*> *childelements;
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            childelements = (*it)->toElement()->getChildrenByFilter(filter, depth - 1);
            elements->insert(elements->end(), childelements->begin(), childelements->end());
        }
    }
    return elements;
}
