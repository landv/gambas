#include "element.h"

bool isLetter(const char *str)
{
    char s = *str;
    if(s >= 65 && s <= 90) return true;//Majuscules
    if(s >= 97 && s <= 122) return true;//Minuscules
    if(s >= 48 && s <= 57) return true; //Chiffres
    return false;
}

wstring Right(wstring str, wstring::size_type len)
{
    return str.substr(str.length() - len, len);
}

vector<wstring>* split(wstring str, wstring pattern)
{
    vector<wstring> *splits = new vector<wstring>;
    wstring s;
    unsigned int i, pos = 0;
    for(i = 0; i < str.length(); i++)
    {
        s = str.at(i);
        if(s == pattern)
        {
            splits->push_back(str.substr(pos, i - pos));
            pos = i + 1;
        }
    }

    if(i > pos)//Il en reste un au bout
    {
        splits->push_back(str.substr(pos, i - pos));
    }
    return splits;
}

wstring Trim(wstring str)
{
    wstring s;
    wstring::size_type i, j;
    for(i = 0; i < str.length(); i++)
    {
        s = str.at(i);
        if(s != L" ") break;
    }

    for(j = str.length() - 1; j > 0;j--)
    {
        s = str.at(j);
        if(s != L" ") break;
    }

    j++;

    return str.substr(i, j - i);

}

bool isLetter(wstring &s){return isLetter(WStringToString(s).c_str());}

bool exist(vector<wstring> vect, wstring elmt)
{
    for(unsigned int i = 0; i < vect.size(); i++)
    {
        if(vect[i] == elmt) return true;
    }
    return false;
}

bool exist(vector<wstring> *vect, wstring elmt)
{
    for(unsigned int i = 0; i < vect->size(); i++)
    {
        if(vect->at(i) == elmt) return true;
    }
    return false;
}

void Element::removeChild(Node *child)
{
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it) == child)
        {
            Node *tNode = *it;
            (*it)->setParent(0);
            (*it)->setOwnerDocument(0);
            GB.Unref(POINTER(&tNode));
            children->erase(it);
            return;
        }
    }
}

bool Element::insertAfter(Node *child, Node *newChild)
{
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it) == child)
        {
            children->insert(++it, newChild);
            newChild->setParent(this);
            newChild->setOwnerDocument(ownerDoc);
            GB.Ref(newChild);
            return true;
        }
    }
    return false;
}

bool Element::insertBefore(Node *child, Node *newChild)
{
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it) == child)
        {
            children->insert(it, newChild);
            newChild->setParent(this);
            newChild->setOwnerDocument(ownerDoc);
            GB.Ref(newChild);
            return true;
        }
    }
    return false;
}

void Element::replaceChild(Node *oldChild, Node *newChild)
{
    if(insertBefore(oldChild, newChild))
        removeChild(oldChild);
}

wstring Element::Virtual::toString(int indent)
{
    wstring str;
    if(indent > 0){str += wstring(indent, ' ');};
    str += L"<"+ parent->getTagName();
    if(parent->attributes->size() > 0){
        for(map<wstring, wstring>::iterator it = parent->attributes->begin(); it != parent->attributes->end(); ++it)
        {
            str += L" " + it->first + L"=\"" + Html$(it->second) + L"\"";
        }}
    str += L">";
    if(indent >= 0) str += L"\n";

    for(list<Node*>::iterator it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        str += (*it)->virt->toString(indent >= 0 ? (indent + 1) : -1);
    }

    if(indent > 0){str += wstring(indent, ' ');};
    str += L"</"+parent->getTagName()+L">";
    if(indent >= 0) str += L"\n";

    return str;
}

void Element::ClearElements()
{
    Node *tnode;
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
            tnode = *it; GB.Unref(POINTER(&tnode));
    }
    children->clear();
}

Node* Element::appendChild(Node *newChild)
{
    children->push_back(newChild);
    newChild->setParent(this);
    newChild->setOwnerDocument(ownerDoc);
    GB.Ref(newChild);
    return newChild;
}

Node* Element::appendChild(Node &newChild)
{
    return appendChild(&newChild);
}

Node* Element::prependChild(Node *newChild)
{
    children->push_front(newChild);
    newChild->setParent(this);
    newChild->setOwnerDocument(ownerDoc);
    GB.Ref(newChild);
    return newChild;
}

Node* Element::prependChild(Node &newChild)
{
    return prependChild(&newChild);
}

wstring Element::Virtual::textContent()
{
    wstring str;
    if(parent->children->size() <= 0) return L"";
    for(list<Node*>::iterator it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        str += (*it)->virt->textContent();
    }

    return str;
}

void Element::Virtual::setTextContent(wstring content)
{
    parent->ClearElements();
    parent->appendText(content);
}

void Element::appendText(wstring text)
{
    TextNode *node = GBI::New<TextNode>("XmlTextNode");
    node->setTextContent(text);
    appendChild(node);
}

void Element::appendFromText(wstring data, bool force)
{
    vector<Node*> *elements = fromText(data, force);
    for(unsigned int i = 0; i < elements->size(); i++)
    {
        this->appendChild(elements->at(i));
    }
    delete elements;
}

inline void increment( wstring::size_type &i, unsigned int &l, unsigned int &c, wstring &s, wstring &data)
{
    i++; c++; s = data[i]; if(s == L"\n" || s == L"\r"){l++; c=1;}
}

inline void increment( wstring::size_type &i, unsigned int &l, unsigned int &c, wstring &s, wstring &data, wstring::size_type num)
{
    for(uint j = 0; j < num; j++)
    {
        increment(i, l, c, s, data);
    }
}



vector<Node*>* Element::fromText(wstring data, wstring::size_type i, uint c, uint l)
{
    vector<Node*>* elements = new vector<Node*>;
    Element *curElement = 0;

    wstring s, text, tag;

    #define INC increment(i, l, c, s, data)
    #define INCS(num) increment(i, l, c, s, data, num)
#define NEAR data.substr(i, Mini<wstring::size_type>(data.length() - i, (80)))
#undef CLEAR
#define CLEAR Node *dnode; for(vector<Node*>::iterator it = elements->begin(); it != elements->end(); ++it)\
    {\
        dnode = *it; GB.Unref(POINTER(&dnode));\
    }\

    #define APPEND(elmt) if(curElement == 0){elements->push_back(elmt);}\
    else {curElement->appendChild(elmt);}

    s = data[i];
    //On commence
    for(; i < data.length(); INC)
    {
        if(s == L"<")//On trouve un début d'élément
        {
            if(text.length() > 0)//On se débarrase du texte
            {
                TextNode *node = GBI::New<TextNode>("XmlTextNode");
                node->setTextContent(text);
                APPEND(node)
                text.erase();
            }

            for(INC; i < data.length(); INC) //On cherche le tagName
            {
                if(!isLetter(s) && s != L":" && s != L"-"&& s != L"_") break;
                tag += s;
            }

            if(tag.length() <= 0)//Il y a quelque chose ...
            {
                if(s == L"/")//C'est un élément de fin
                {
                    if(curElement == 0) {i -=2; c-=2; CLEAR throw HTMLParseException(c, l, NEAR, L"Unexpected end tag.");}
                    INC;
                    if(!(data.substr(i, curElement->getTagName().length()) == curElement->getTagName()))
                    {//Ce n'est pas le bon

                            i -=2; c-=2; CLEAR throw HTMLParseException(c, l, NEAR, L"Unexpected end tag.");

                    }

                    INCS(curElement->getTagName().length());
                    curElement = curElement->getParent();//On a terminé, on remonte
                    continue;
                }
                else if(data.substr(i, 3) == L"!--")//C'est un commentaire
                {
                    INCS(3);
                    wstring comm;
                    for(; i<data.length(); INC)
                    {
                        if(data.substr(i, 3) == L"-->") break;//Fin du commentaire
                        comm += s;
                    }
                    CommentNode *comment = GBI::New<CommentNode>("XmlCommentNode");
                    comment->setTextContent(comm);
                    APPEND(comment)
                    INCS(2);
                    continue;
                }
                else if(data.substr(i, 8) == L"![CDATA[")//Structrure CDATA
                {
                    INCS(8);
                    wstring comm;
                    for(; i<data.length(); i++)
                    {
                        s = data[i];
                        if(data.substr(i, 3) == L"]]>") break;//Fin du CDATA
                        comm += s;
                    }
                    CDATANode *comment = GBI::New<CDATANode>("XmlCDATANode");
                    comment->setTextContent(comm);
                    APPEND(comment)
                    INCS(2);
                    continue;
                }
                else//Euuh ...
                {
                    CLEAR
                    throw HTMLParseException(c, l, NEAR, L"Unexpected character in tag.");
                }
            }

            //Si tout va bien, on a un nouvel élément
            Element *elmt = GBI::New<Element>("XmlElement");
            elmt->setTagName(tag);
            APPEND(elmt);
            curElement = elmt;

            while(i<data.length())//On gère le contenu
            {
                if(s == L">") break; //Fin de l'élément
                if(s == L"/") //Élément auto-fermant
                {
                    INC;
                    curElement = curElement->getParent();//Pas d'enfants, on remonte
                    break;
                }

                wstring attr, sVal = L"";

                if(isLetter(s))
                {
                    while(i < data.length() && (isLetter(s) || s == L"-" || s == L":" || s == L"_"))
                    {
                        attr += s;
                        INC;
                    }
                    if(s != L"=")
                    {
                        i -= attr.length();
                        CLEAR
                        throw HTMLParseException(c, l,  NEAR,  L"Expected '=' after attribute name.");
                    }

                    INC;
                    wstring delimiter = s;

                    if(delimiter != L"\"" && delimiter != L"'"){//Pas de délimiteur
                        CLEAR
                        throw HTMLParseException(c, l, NEAR,  L"Expected delimiter.");
                    }

                    INC;

                    while(i<data.length() && s != delimiter)
                    {
                        if(s == L"\\")//Échappement
                        {
                            INC; //On avance
                            sVal += L"\\";//On garde quand même le backslash
                        }
                            sVal += s;
                        INC;
                    }

                    curElement->setAttribute(attr, sVal);
                }
                INC;
            }

            tag.erase();


        }
        else if(s == L">")//Erreur
        {
            CLEAR throw HTMLParseException(c, l,NEAR,   L"Unexpected '>'.");
        }
        else//Texte normal
        {
            if(s == L"&")
            {
                if(data.substr(i, 5) == L"&amp;")
                {
                    text += L"&";
                    INCS(4);
                }
                else if(data.substr(i, 4) == L"&lt;")
                {
                    text += L"<";
                    INCS(3);
                }
                else if(data.substr(i, 4) == L"&gt;")
                {
                    text += L">";
                    INCS(3);
                }
                else if(data.substr(i, 6) == L"&quot;")
                {
                    text += L"\"";
                    INCS(5);
                }
                else
                {
                    //throw HTMLParseException(1, i, "Unexpected '&'.");
                    text += L"&";
                }
            }
            else
            {
                text += s;
            }
        }
    }

    if(text.length() > 0)//On évacue le texte qui reste à la fin
    {
        TextNode *node = GBI::New<TextNode>("XmlTextNode");
        node->setTextContent(text);
        APPEND(node)
    }

    return elements;
    #undef APPEND
}

GBI::ObjectArray<Element>* Element::getChildrenByAttributeValue(wstring attr, wstring val, int depth)
{
    GBI::ObjectArray<Element> *elements = new GBI::ObjectArray<Element>("XmlElement");
    if(depth == 0) return elements;
    if(getAttribute(attr) == val) elements->push_back(this);
    if(depth == 1) return elements;
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            elements->push_back((*it)->toElement()->getChildrenByAttributeValue(attr, val, depth - 1));
        }
    }
    return elements;
}

GBI::ObjectArray<Element>* Element::getGBChildrenByTagName(wstring tag, int depth)
{
    GBI::ObjectArray<Element> *elements = new GBI::ObjectArray<Element>("XmlElement");
    if(depth == 0) return elements;
    if(*tagName == tag) elements->push_back(this);
    if(depth == 1) return elements;
    vector<Element*> *childelements;
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            childelements = (*it)->toElement()->getChildrenByTagName(tag, depth - 1);
            elements->push_back(childelements);
            delete childelements;
        }
    }
    return elements;
}

vector<Element*>* Element::getChildrenByTagName(wstring tag, int depth)
{
    vector<Element*> *elements = new vector<Element*>;
    if(depth == 0) return elements;
    if(*tagName == tag) elements->push_back(this);
    if(depth == 1) return elements;
    vector<Element*> *childelements;
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            childelements = (*it)->toElement()->getChildrenByTagName(tag, depth - 1);
            elements->insert(elements->end(), childelements->begin(), childelements->end());
            delete childelements;
        }
    }
    return elements;
}

wstring Element::getAttribute(wstring key)
{
    try{return attributes->at(key);}
    catch(out_of_range &e)
    {
        return L"";
    }
}

GBI::ObjectArray<Node>* Element::getAllChildren()
{
    GBI::ObjectArray<Node> *allchildren = new GBI::ObjectArray<Node>("XmlNode");
    for(list<Node*>::iterator it = children->begin(); it != children->end(); ++it)
    {
        allchildren->push_back(*it);
        if((*it)->isElement())
        {
            allchildren->push_back((*it)->toElement()->getAllChildren());
        }
    }
    return allchildren;
}

void Element::Virtual::setOwnerDocument(Document *doc)
{
    parent->ownerDoc = doc;
    for(auto it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        (*it)->virt->setOwnerDocument(doc);
    }
}

GBI::ObjectArray<Element>* Element::getChildElements()
{
    GBI::ObjectArray<Element>* vect = new GBI::ObjectArray<Element>("XmlElement");
    for(auto it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            vect->push_back((*it)->toElement());
        }
    }
    return vect;
}

GBI::ObjectArray<Node>* Element::getGBChildren()
{
    GBI::ObjectArray<Node> *childs = new GBI::ObjectArray<Node>("XmlNode");
    for(auto it = children->begin(); it != children->end(); ++it)
    {
        childs->push_back(*it);
    }
    return childs;
}


Element* Element::getFirstChildByTagName(wstring tag, int depth)
{
    Element *elmt = 0;
    if(depth == 0) return 0;
    if(this->getTagName() == tag) return this;
    if(depth == 1) return 0;
    for(auto it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            elmt = (*it)->toElement()->getFirstChildByTagName(tag, depth - 1);
            if(elmt) return elmt;
        }
    }
    return 0;
}


Element* Element::previousSibling()
{
    Element *elmt = 0;
    if(!parent) return 0;
    if(!parent->children->size()) return 0;
    list<Node*>::iterator it;
    for(it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        if(*it == this) break;
    }

    if(it == parent->children->begin()) return 0; //Si c'est le premier, y risque pas d'y en avoir avant

    while(it != (parent->children->begin()))
    {
        --it;
        if((*it)->isElement())
        {
            elmt = (*it)->toElement();
            break;
        }
    }

    return elmt;

}

Element* Element::nextSibling()
{
    Element *elmt = 0;
    if(!parent) return 0;
    if(!parent->children->size()) return 0;
    list<Node*>::iterator it;
    for(it = parent->children->begin(); it != parent->children->end(); ++it)
    {
        if(*it == this) break;
    }

    if(it == --parent->children->end()) return 0; //Si c'est le dernier, y risque pas d'y en avoir avant

    while(it != (parent->children->end()))
    {
        ++it;
        if((*it)->isElement())
        {
            elmt = (*it)->toElement();
            break;
        }
    }

    return elmt;

}


Element* Element::firstChildElement()
{
    for(auto it = children->begin(); it != children->end(); ++it)
    {
        if((*it)->isElement())
        {
            return (*it)->toElement();
        }
    }
    return 0;
}

Element* Element::lastChildElement()
{
    for(auto it = --(children->end()); it != --(children->begin()); ++it)
    {
        if((*it)->isElement())
        {
            return (*it)->toElement();
        }
    }
    return 0;
}

bool Element::isAttributeSet(wstring key)
{
        try{attributes->at(key);}
        catch(out_of_range &e)
        {
            return false;
        }
    return true;
}

bool Element::MatchXPathFilter(wstring filter)
{
#define CHECKPOS if(pos >= filter.length() - 1)\
    {\
        GB.Error("Invalid Filter");\
        return false;\
    }

    wstring::size_type pos;
    wstring s;

    filter = Trim(filter);


    if(filter == L"*") return true;

    pos = filter.rfind(L"|");
    if(pos != wstring::npos)
    {
        CHECKPOS
        return MatchXPathFilter(filter.substr(pos +  1)) || MatchXPathFilter(filter.substr(0, pos));
    }

    pos = filter.rfind(L"/");
    if(pos != wstring::npos)
    {
        CHECKPOS
        if(pos == 0)//Si le / est au début (chemin absolu)
        {
            return !(this->parent) && MatchXPathFilter(filter.substr(1));
        }

        s = filter.at(pos - 1);
        if(s == L"/")//Si on a un double slash //
        {
            if(!MatchXPathFilter(filter.substr(pos + 1))) return false;
            Element *elmt = this->parent;
            while(elmt)
            {
                if(elmt->MatchXPathFilter(filter.substr(0, pos - 1))) return true;
                elmt = elmt->parent;
            }

            return false;
        }

      if(!(this->parent)) return false;
        return MatchXPathFilter(filter.substr(pos +  1)) && this->parent->MatchXPathFilter(filter.substr(0, pos));
    }

    return MatchSubXPathFilter(filter);
}

bool Element::MatchSubXPathFilter(wstring filter)
{
    if(filter == L"*") return true;
    if(filter == *(this->tagName))
    {
        return true;
    }
    return false;
}

Node* Element::Virtual::cloneNode()
{
    Element *node = GBI::New<Element>("XmlElement");
    node->setTagName(*(parent->tagName));
    return node;
}
