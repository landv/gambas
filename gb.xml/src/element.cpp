#include "element.h"
#include "CElement.h"

vector<fwstring> Element::singleElements = {"br", "hr", "area", "base", "br", "co",
                                          "command", "embed", "hr", "img", "input", "keygen",
                                          "link", "meta", "param", "source", "track", "wbr"};
Element::Element()
{
    tagName = new fwstring;
    firstChild = 0;
    lastChild = 0;
    childCount = 0;
    //DEBUG << "newelmt, childlist : " << children << endl;
    attributes = new flist<AttrListElement*>;

    attributeNode = new AttrNode;
    attributeNode->attrName = 0;
    relElmt = 0;

}

Element::~Element()
{

    delete attributes;
    attributes = 0;
}

bool isLetter(const char *str)
{
    char s = *str;
    if(s >= 65 && s <= 90) return true;//Majuscules
    if(s >= 97 && s <= 122) return true;//Minuscules
    if(s >= 48 && s <= 57) return true; //Chiffres
    return false;
}

fwstring Right(fwstring str, size_t len)
{
    return (str.substr(str.length() - len, len));
}

vector<fwstring>* split(fwstring str, fwstring pattern)
{
    vector<fwstring> *splits = new vector<fwstring>;
    fwstring s;
    unsigned int i, pos = 0;
    for(i = 0; i < str.length(); i++)
    {
        s = str.at(i);
        if(s == pattern)
        {
            splits->push_back((str.substr(pos, i - pos)));
            pos = i + 1;
        }
    }

    if(i > pos)//Il en reste un au bout
    {
        splits->push_back((str.substr(pos, i - pos)));
    }
    return splits;
}

fwstring Trim(fwstring str)
{
    fwstring s;
    fwstring::size_type i, j;
    for(i = 0; i < str.length(); i++)
    {
        s = str.at(i);
        if(s != " ") break;
    }

    for(j = str.length() - 1; j > 0;j--)
    {
        s = str.at(j);
        if(s != " ") break;
    }

    j++;

    return (str.substr(i, j - i));

}

bool isLetter(fwstring &s){return isLetter((s).c_str());}

bool exist(vector<fwstring> vect, fwstring elmt)
{
    for(unsigned int i = 0; i < vect.size(); i++)
    {
        if(vect[i] == elmt) return true;
    }
    return false;
}

bool exist(vector<fwstring> *vect, fwstring elmt)
{
    for(unsigned int i = 0; i < vect->size(); i++)
    {
        if(vect->at(i) == elmt) return true;
    }
    return false;
}

void Element::removeChild(Node *child)
{
    if(child == firstChild) firstChild = child->nextNode;
    if(child == lastChild) lastChild = child->previousNode;
    if(child->nextNode) child->nextNode->previousNode = child->previousNode;
    if(child->previousNode) child->previousNode->nextNode = child->nextNode;
    childCount--;
}

bool Element::insertAfter(Node *child, Node *newChild)
{
    if(child->parent != this) return false;
    newChild->nextNode = child->nextNode;
    newChild->previousNode = child;
    if(child->nextNode)
    {
        child->nextNode->previousNode = newChild;
    }
    if(child == lastChild)
    {
        lastChild = newChild;
    }
    child->nextNode = newChild;
    childCount++;
    return true;
}

bool Element::insertBefore(Node *child, Node *newChild)
{
    if(child->parent != this) return false;
    newChild->nextNode = child;
    newChild->previousNode = child->previousNode;
    if(child->previousNode)
    {
        child->previousNode->nextNode = newChild;
    }
    if(child == firstChild)
    {
        firstChild = newChild;
    }
    child->previousNode = newChild;
    childCount++;
    return true;
}

void Element::replaceChild(Node *oldChild, Node *newChild)
{
    if(insertBefore(oldChild, newChild))
        removeChild(oldChild);
}
/*
#ifndef HELEMENT_CPP

fwstring Element::Virtual::toString(int indent)
{
    fwstring str;
    if(indent > 0){str += fwstring(indent, ' ');};
    str += "<"+ parent->getTagName();
    if(parent->attributes->size() > 0){
        for(map<fwstring, fwstring>::iterator it = parent->attributes->begin(); it != parent->attributes->end(); ++it)
        {
            str += " " + it->first + "=\"" + Html$(it->second) + "\"";
        }}
    str += ">";
    if(indent >= 0) str += "\n";

    for(list<Node*>::iterator it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        str += (*it)->virt->toString(indent >= 0 ? (indent + 1) : -1);
    }

    if(indent > 0){str += fwstring(indent, ' ');};
    str += "</"+parent->getTagName()+">";
    if(indent >= 0) str += "\n";

    return str;
}

#else

*/
fwstring Element::toString(int indent)
{
    fwstring str;
    if(indent > 0){str += fwstring(indent, ' ');};
    str += fwstring("<");
    //DEBUG << str.toStdString() << endl;
    str += getTagName();
    //DEBUG << str.toStdString() << endl;
    //DEBUGH;
    if(attributes->len > 0){
        //DEBUGH;
    for(flist<AttrListElement*>::element *it = attributes->firstElement; it != 0; it = it->next)
    {
        //DEBUGH;
        str += " ";
        str += *(it->value->attrName);
        str += "=\"";
        str += *(it->value->attrValue);
        str += "\"";
    }}

    //DEBUGH;

    if(exist(Element::singleElements, getTagName()))
    {
        str += " />";
    }
    else
    {
        str += ">";
        if(indent >= 0) str += "\n";

        for(Node *it = firstChild; it != 0; it = it->nextNode)
        {
            str += (it)->toString(indent >= 0 ? (indent + 1) : -1);
        }

        if(indent > 0){str += fwstring(indent, ' ');};
        str += "</";
        str += getTagName();
        ///DEBUG << getTagName().toStdString() << endl;
        str += ">";
        if(indent >= 0) str += "\n";
    }
    //DEBUGH;
    return str;
}


void Element::ClearElements()
{
    /*Node *tnode;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
            tnode = it->value; //GB.Unref(POINTER(&tnode));
    }
    children->clear();*/
    firstChild = 0;
    lastChild = 0;
}

Node* Element::appendChild(Node *newChild)
{
    //children->push_back(newChild);
    childCount++;
    if(!lastChild)//La liste est vide
    {
        firstChild = newChild;
        lastChild = firstChild;
        lastChild->nextNode = 0;
        lastChild->previousNode = 0;
        return newChild;
    }

    newChild->previousNode = lastChild;
    newChild->nextNode = 0;
    lastChild->nextNode = newChild;
    lastChild = newChild;

    newChild->setParent(this);
    newChild->setOwnerDocument(ownerDoc);
    //GB.Ref(newChild);
    return newChild;
}

Node* Element::appendChild(Node &newChild)
{
    return appendChild(&newChild);
}

Node* Element::prependChild(Node *newChild)
{
    //children->push_front(newChild);

    childCount++;
    if(!lastChild)//La liste est vide
    {
        firstChild = newChild;
        lastChild = firstChild;
        lastChild->nextNode = 0;
        lastChild->previousNode = 0;
        return newChild;
    }

    newChild->nextNode = firstChild;
    newChild->previousNode = 0;
    firstChild->previousNode = newChild;
    firstChild = newChild;

    newChild->setParent(this);
    newChild->setOwnerDocument(ownerDoc);
    //GB.Ref(newChild);
    return newChild;
}

Node* Element::prependChild(Node &newChild)
{
    return prependChild(&newChild);
}

fwstring Element::textContent()
{
    fwstring str;
    if(childCount == 0) return "";
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        str += (it)->textContent();
    }

    return str;
}

void Element::setTextContent(fwstring content)
{
    ClearElements();
    appendText(content);
}

void Element::appendText(fwstring text)
{
    TextNode *node = new TextNode;
    node->setTextContent(*(text.copy()));
    appendChild(node);
}

void Element::appendFromText(fwstring data, bool force)
{
    fvector<Node*> *elements = fromText(data, force);
    for(unsigned int i = 0; i < elements->size(); i++)
    {
        this->appendChild(elements->at(i));
    }
    delete elements;
}

inline void increment( fwstring::size_type &i, unsigned int &l, unsigned int &c, wchar_t &s, fwstring &data)
{
    i++; c++; s = data[i]; if(s == SCHAR_N || s == SCHAR_R){l++; c=1;}
}

inline void increment( fwstring::size_type &i, unsigned int &l, unsigned int &c, fwstring &s, fwstring &data, fwstring::size_type num)
{

}

fvector<Node*>* Element::fromText(fwstring data, size_t start)
{
    fvector<Node*>* elements = new fvector<Node*>;//Liste des éléments à retourner

    if(!data.len) return elements; //Chaîne vide ?

    Element *curElement = 0;//Élément courant

#define APPEND(elmt) if(curElement == 0){elements->push_back(elmt);  /*DEBUG << elements->at(0) << endl;*/}\
else {curElement->appendChild(elmt);}//Ajoute 'elmt' à la liste

#define INC i++; s = source[i]//Passe au byte suivant
    size_t i = start;//Byte courant (indice)
    register char s = 0;//Byte courant (valeur)
    char *source = data.data;//Chaîne source
    size_t len = data.len;//Longueur de la source (en bytes)
    register wchar_t ws = 0;//Caractère courant (valeur)

    size_t tagPos = 0;//Position du premier caractère < trouvé
    char *tag = 0;//Premier caractère < trouvé

    //DEBUG << i << endl;
    //DEBUG << len << endl;

    while(i < len)//On commence
    {
        s = source[i];
        //DEBUG << i << endl;
        tag = (char*)memchr((source + i), CHAR_STARTTAG, len - i);//On cherche un début de tag
        tagPos = (tag - source);
        //DEBUG << (void*)tag << " - " << (void*)source <<  " = " << tagPos << endl;
        if(tag)//On ajoute le texte, s'il existe
        {
            TextNode *text = new TextNode;
            text->content = data.copyString(i, tagPos);
            APPEND(text);
        }

        if(!tag) break; //Pas de tag suivant ? On a fini !
        i = tagPos + 1;//On avance au caractère trouvé

        //DEBUG << i << endl;

        //On analyse le contenu du tag
        ws = data.increment(i);//On prend le premier caractère

        if(!isNameStartChar(ws))//Ce n'est pas un tagName, il y a quelque chose ...
        {
            if(ws == CHAR_SLASH)//C'est un élément de fin
            {
                if(!curElement)//Pas d'élément courant
                {
                    //ERREUR : CLOSING TAG WHEREAS NONE IS OPEN
                    throw HTMLParseException(0, 0, "", "CLOSING TAG WHEREAS NONE IS OPEN");
                }
                if((len - i) < curElement->tagName->len)//Impossible que les tags correspondent
                {
                    //ERREUR : TAG MISMATCH
                    throw HTMLParseException(0, 0, "", "TAG MISMATCH");
                }
                //Les tags ne correspondent pas
                else if(memcmp(source + i, curElement->tagName->data, curElement->tagName->len) != 0)
                {
                    //ERREUR : TAG MISMATCH
                    //DEBUG << *(curElement->tagName) << " " << fwstring(source + i, curElement->tagName->len) << endl;
                    throw HTMLParseException(0, 0, "", "TAG MISMATCH");
                }
                else//Les tags correspondent, on remonte
                {
                    i += curElement->tagName->len;
                    //DEBUG << i << endl;
                    curElement = curElement->parent;
                    tag = (char*)memchr((source + i), CHAR_ENDTAG, len - i);//On cherche la fin du tag
                    tagPos = (tag - source);
                    //DEBUG << (void*)tag <<"-"<< (void*)source << endl;
                    i = tagPos +1;//On avance à la fin du tag

                    //DEBUG << i << endl;
                    continue;
                }
            }
            else if(ws == CHAR_EXCL)//Ce serait un commentaire ou un CDATA
            {
                if(memcmp(source + i, "--", 2) == 0)//C'est bien un commentaire
                {
                    i += 2;//On va au début du contenu du commentaire
                    tag = (char*)memchrs(source + i, len - i, "-->", 3);
                    if(!tag)//Commentaire sans fin
                    {
                        //ERREUR : UNENDED COMMENT
                        throw HTMLParseException(0, 0, "", "UNENDED COMMENT");
                    }

                    tagPos = tag - source;

                    CommentNode *comment = new CommentNode;
                    comment->content = data.copyString(i, tagPos);
                    APPEND(comment);

                    i = tagPos + 3;
                    continue;
                }
                else if(memcmp(source + i + 1, "[CDATA[", 7) == 0)//C'est un CDATA
                {
                    i += 3;//On va au début du contenu du cdata
                    tag = (char*)memchrs(source, len - i, "]]>", 3);
                    if(!tag)//Cdata sans fin
                    {
                        //ERREUR : UNENDED CDATA
                        throw HTMLParseException(0, 0, "", "UNENDED CDATA");
                    }

                    tagPos = tag - source;

                    CDATANode *cdata = new CDATANode;
                    cdata->content = data.copyString(i, tagPos);
                    APPEND(cdata);
                    continue;
                }
                else// ... ?
                {
                    //ERREUR : INVALID TAG
                    //DEBUG << "TxtPos : " << source + i + 1 << endl;
                    throw HTMLParseException(i, 0, "", "INVALID TAG");
                }
            }
            else// ... ?
            {
                //ERREUR : INVALID TAG
                //DEBUG << "TagPos : " << tagPos << endl;
                throw HTMLParseException(i, 0, "", "INVALID TAG");
            }
        }//Si tout va bien, on a un nouvel élément
        else
        {
            while(isNameChar(data.increment(i)))//On cherche le tagName
            {
                if(i > len)
                {
                    //ERREUR : UNENDED TAG
                    throw HTMLParseException(0, 0, "", "UNENDED TAG");
                }
            }
            i--;

            Element *elmt = new Element;
            elmt->tagName = data.copyString(tagPos + 1, i);
            //DEBUG << tagPos << endl;
            APPEND(elmt);
            curElement = elmt;
            s = source[i];
            //DEBUG << "S : " << s << endl;

            //DEBUG << "TAG : " << *(elmt->tagName) << endl;

            while(i < len)//On gère le contenu de l'élément (attributs)
            {
                if(s == CHAR_ENDTAG) break;//Fin de l'élément
                if(s == CHAR_SLASH) //Élément auto-fermant
                {
                    i++;
                    curElement = curElement->getParent();//Pas d'enfants, on remonte
                    break;
                }

                if(isNameStartChar(s))//Début d'attribut
                {
                    size_t attrNamestart = i;
                    while(isNameChar(data.increment(i)) && i < len){}//On parcourt le nom d'attribut
                    i--;
                    size_t attrNameEnd = i;
                    s = source[i];
                    //DEBUG << "S : " << s << endl;
                    //DEBUG << "ATTR : " << *(data.copyString(attrNamestart, attrNameEnd)) << endl;

                    while(isWhiteSpace(s) && i < len){INC;}//On ignore les espaces blancs

                    if(s != CHAR_EQUAL)
                    {
                        elmt->setAttribute(*(data.ssubstr(attrNamestart, attrNamestart - attrNameEnd)), "");
                        if(s == CHAR_ENDTAG) break;//Fin de l'élément
                        else if (s == CHAR_SLASH)//Élément auto-fermant
                        {
                            i++;
                            curElement = curElement->getParent();//Pas d'enfants, on remonte
                            break;
                        }
                        else
                        {
                            //ERREUR : INVALID TAG
                            //DEBUG << "S : " << s << endl;
                            throw HTMLParseException(i, 0, "", "INVALID TAG");
                        }
                    }

                    INC;

                    while(isWhiteSpace(s) && i < len){INC;}//On ignore les espaces blancs

                    //DEBUG << "S : " << s << endl;
                    char delimiter = s;
                    if(delimiter != CHAR_DOUBLEQUOTE && delimiter != CHAR_SINGLEQUOTE)
                    {
                        //ERREUR : EXPECTED ATTRIBUTE DELIMITER
                        throw HTMLParseException(0, 0, "", "EXPECTED ATTRIBUTE DELIMITER");
                    }
                    i++;

                    size_t delimiterPos = (char*)memchr(source + i, delimiter, len - i) - source;

                    elmt->addAttribute(*(data.ssubstr(attrNamestart, attrNameEnd - attrNamestart)),
                                       *(data.ssubstr(i, delimiterPos - i)));
                    i = delimiterPos;

                }

                INC;
            }

        }
        i++;

    }

    //DEBUG << i << endl;

    return elements;

}

GBI::ObjectArray<Element>* Element::getChildrenByAttributeValue(fwstring attr, fwstring val, int depth)
{
    GBI::ObjectArray<Element> *elements = new GBI::ObjectArray<Element>("XmlElement");
    if(depth == 0) return elements;
    if(getAttribute(attr) == val) elements->push_back(this);
    if(depth == 1) return elements;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            elements->push_back((it)->toElement()->getChildrenByAttributeValue(attr, val, depth - 1));
        }
    }
    return elements;
}

GBI::ObjectArray<Element>* Element::getGBChildrenByTagName(fwstring tag, int depth)
{
    GBI::ObjectArray<Element> *elements = new GBI::ObjectArray<Element>("XmlElement");
    if(depth == 0) return elements;
    if(tagName->toString() == tag) elements->push_back(this);
    if(depth == 1) return elements;
    vector<Element*> *childelements;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            childelements = (it)->toElement()->getChildrenByTagName(tag, depth - 1);
            elements->push_back(childelements);
            delete childelements;
        }
    }
    return elements;
}

vector<Element*>* Element::getChildrenByTagName(fwstring tag, int depth)
{
    vector<Element*> *elements = new vector<Element*>;
    if(depth == 0) return elements;
    if(tagName->toString() == tag) elements->push_back(this);
    if(depth == 1) return elements;
    vector<Element*> *childelements;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            childelements = (it)->toElement()->getChildrenByTagName(tag, depth - 1);
            elements->insert(elements->end(), childelements->begin(), childelements->end());
            delete childelements;
        }
    }
    return elements;
}

fwstring Element::getAttribute(fwstring key)
{
    for(flist<AttrListElement*>::element *it = attributes->firstElement; it != 0; it = it->next)
    {
        if(*(it->value->attrName) == key)
        {
            return *(it->value->attrValue);
        }
    }
    return "";
}

void Element::setAttribute(fwstring key, fwstring value)
{
    //DEBUG << this << " " << attributes << endl;
    for(flist<AttrListElement*>::element *it = attributes->firstElement; it != 0; it = it->next)
    {
        if(*(it->value->attrName) == key)
        {
            it->value->attrValue = value.copy();
            return;
        }
    }

    addAttribute(key, value);

}

void Element::addAttribute(fwstring key, fwstring value)
{
    AttrListElement *newAttr = (AttrListElement*)malloc(sizeof(AttrListElement));
    newAttr->attrName = key.copy();
    newAttr->attrValue = value.copy();
    attributes->push_back(newAttr);
}

GBI::ObjectArray<Node>* Element::getAllChildren()
{
    GBI::ObjectArray<Node> *allchildren = new GBI::ObjectArray<Node>("XmlNode");
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        allchildren->push_back(it);
        if((it)->isElement())
        {
            allchildren->push_back((it)->toElement()->getAllChildren());
        }
    }
    return allchildren;
}

void Element::setOwnerDocument(Document *doc)
{
    ownerDoc = doc;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        (it)->setOwnerDocument(doc);
    }
}

GBI::ObjectArray<Element>* Element::getChildElements()
{
    GBI::ObjectArray<Element>* vect = new GBI::ObjectArray<Element>("XmlElement");
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            vect->push_back((it)->toElement());
        }
    }
    return vect;
}

GBI::ObjectArray<Node>* Element::getGBChildren()
{
    GBI::ObjectArray<Node> *childs = new GBI::ObjectArray<Node>("XmlNode");
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        childs->push_back(it);
    }
    return childs;
}


Element* Element::getFirstChildByTagName(fwstring tag, int depth)
{
    Element *elmt = 0;
    if(depth == 0) return 0;
    if(this->getTagName() == tag) return this;
    if(depth == 1) return 0;
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            elmt = (it)->toElement()->getFirstChildByTagName(tag, depth - 1);
            if(elmt) return elmt;
        }
    }
    return 0;
}


Element* Element::previousSibling()
{
    if(!parent) return 0;

    for(Node *it = this; it != 0; it = it->previousNode)
    {
        if((it)->isElement())
        {
            return it->toElement();
        }
    }

    return 0;

}

Element* Element::nextSibling()
{
    if(!parent) return 0;

    for(Node *it = this; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            return it->toElement();
        }
    }

    return 0;

}


Element* Element::firstChildElement()
{
    for(Node *it = firstChild; it != 0; it = it->nextNode)
    {
        if((it)->isElement())
        {
            return it->toElement();
        }
    }

    return 0;
}

Element* Element::lastChildElement()
{
    for(Node *it = lastChild; it != 0; it = it->previousNode)
    {
        if((it)->isElement())
        {
            return it->toElement();
        }
    }

    return 0;
}

bool Element::isAttributeSet(fwstring key)
{
    for(flist<AttrListElement*>::element *it = attributes->firstElement; it != 0; it = it->next)
    {
        if(*(it->value->attrName) == key)
        {
            return true;
        }
    }
    return false;
}

bool Element::MatchXPathFilter(fwstring filter)
{
#define CHECKPOS if(pos >= filter.length() - 1)\
    {\
        GB.Error("Invalid Filter");\
        return false;\
    }

    fwstring::size_type pos;
    fwstring s;

    filter = Trim(filter);


    if(filter == "*") return true;

    pos = filter.rfind("|");
    if(pos != fwstring::npos)
    {
        CHECKPOS
        return MatchXPathFilter((filter.substr(pos +  1))) || MatchXPathFilter((filter.substr(0, pos)));
    }

    pos = filter.rfind("/");
    if(pos != fwstring::npos)
    {
        CHECKPOS
        if(pos == 0)//Si le / est au début (chemin absolu)
        {
            return !(this->parent) && MatchXPathFilter((filter.substr(1)));
        }

        s = filter.at(pos - 1);
        if(s == "/")//Si on a un double slash //
        {
            if(!MatchXPathFilter((filter.substr(pos + 1)))) return false;
            Element *elmt = this->parent;
            while(elmt)
            {
                if(elmt->MatchXPathFilter((filter.substr(0, pos - 1)))) return true;
                elmt = elmt->parent;
            }

            return false;
        }

      if(!(this->parent)) return false;
        return MatchXPathFilter((filter.substr(pos +  1))) && this->parent->MatchXPathFilter((filter.substr(0, pos)));
    }

    return MatchSubXPathFilter(filter);
}

bool Element::MatchSubXPathFilter(fwstring filter)
{
    if(filter == "*") return true;
    if(filter == (this->tagName)->toString())
    {
        return true;
    }
    return false;
}

Node* Element::cloneNode()
{
    Element *node = new Element;
    node->setTagName(*(tagName));
    return node;
}

fwstring AttrNode::textContent()
{
    if((attrName))
        return parent->getAttribute(*(attrName));

    return "";
}

void AttrNode::setTextContent(fwstring content)
{
    if(attrName)
        parent->setAttribute(*(attrName), content);
}

/*Element* Element::FNew()
{
    if(!Element::ClassName) GBI::InitClasses();
    Element* obj = new Element;
    obj->ref = 0;
    obj->klass = Element::ClassName;
    return obj;
    //return reinterpret_cast<T*>(GB.New(T::ClassName, 0, 0));
}*/

void AttrNode::NewGBObject()
{
    NoInstanciate = true;
    //relElmt = GBI::New<CNode>("_XmlAttrNode");
    //relElmt->node = this;
    //GB.Ref(relob);
    NoInstanciate = false;
}

void Element::NewGBObject()
{
    NoInstanciate = true;
    SetGBObject(GBI::New<CElement>("XmlElement"));
    NoInstanciate = false;
}

void Element::SetGBObject(CElement *ob)
{
    relElmt = ob;
    relElmt->elmt = this;
    relElmt->n.node = this;
}

Node* AttrNode::cloneNode()
{
    AttrNode *newnode = new AttrNode;
    newnode->setAttrName(*attrName);
    newnode->setTextContent(*attrValue);
    return newnode;
}
