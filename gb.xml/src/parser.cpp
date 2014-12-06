#include "parser.h"

#include "node.h"
#include "element.h"
#include "textnode.h"
#include "gbinterface.h"
#include <stdlib.h>


/*****    Parser     *****/

void GBparse(const char *data, const size_t lendata, GB_ARRAY *array, DocumentType docType)
{
    if(docType == HTMLDocumentType || docType == XHTMLDocumentType)
    {
        if(CheckHtmlInterface())
        {
            HTML.GBparseHTML(data, lendata, array);
            return;
        }
    }

    GBparseXML(data, lendata, array);
}

Node** parse(char const *data, const size_t lendata, size_t *nodeCount, DocumentType docType) throw(XMLParseException)
{
    if(docType == HTMLDocumentType || docType == XHTMLDocumentType)
    {
        if(CheckHtmlInterface())
        {
            return HTML.parseHTML(data, lendata, nodeCount);
        }
    }

    return parseXML(data, lendata, nodeCount);
}

void GBparseXML(const char *data, const size_t lendata, GB_ARRAY *array)
{
    size_t nodeCount;
    size_t i = 0;
    Node **nodes = parseXML(data, lendata, &nodeCount);
    GB.Array.New(array, GB.FindClass("XmlNode"), nodeCount);

    for(i = 0; i < nodeCount; ++i)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) =  XMLNode_GetGBObject(nodes[i]);
        GB.Ref(nodes[i]->GBObject);
    }

    free(nodes);
}

//Ajoute 'elmt' à la liste
#define APPEND(_elmt) if(curElement == 0)\
{\
    (*nodeCount)++;\
    elements = (Node**)realloc(elements, sizeof(Node*) * (*nodeCount));\
    elements[(*nodeCount) - 1] = _elmt;\
}\
else \
{\
    XMLNode_appendChild(curElement, _elmt); \
}

#define THROW(_ex) for(size_t i = 0; i < *nodeCount; i++)\
{\
    XMLNode_Free(elements[i]);\
}\
free(elements);\
throw(_ex)

Node** parseXML(char const *data, const size_t lendata, size_t *nodeCount) throw(XMLParseException)
{
    *nodeCount = 0;
    if(!lendata || !data) return 0; //Empty ?

    const char *endData = data + lendata;

    Node **elements = 0;//Elements to return
    Element *curElement = 0;//Current element


    register char s = 0;//Current byte (value)
    register char const *pos = data;//Current byte (position)
    register wchar_t ws = 0;//Current character (value)

    char *tag = 0;//First '<' character found

    while(pos < endData)//Start
    {
        tag = (char*)memchr(pos, '<', endData - pos);//On cherche un début de tag
        if(tag && (tag - pos) != 0)//On ajoute le texte, s'il existe
        {
            //Checking length
            char const *textpos = pos;
            size_t textlen = tag - pos;
            //Trim(textpos, textlen);
            if(textlen != 0)
            {
                TextNode *text = XMLTextNode_New();
                XMLTextNode_setEscapedTextContent(text, textpos, textlen);
                APPEND(text);
            }
        }

        if(!tag)
        {
            if(pos < endData)//Il reste du texte
            {
                //Checking length
                char const *textpos = pos;
                size_t textlen = endData - pos;
                //Trim(textpos, textlen);
                if(textlen != 0)
                {
                    TextNode *text = XMLTextNode_New();
                    XMLTextNode_setEscapedTextContent(text, textpos, textlen);
                    APPEND(text);
                }
            }
            break;
        }

        tag++;
        pos = tag;//On avance au caractère trouvé

        //On analyse le contenu du tag
        ws = nextUTF8Char(pos, endData - pos);//On prend le premier caractère

        if(!isNameStartChar(ws))//Ce n'est pas un tagName, il y a quelque chose ...
        {
            if(ws == '/')//C'est un élément de fin
            {
                if(!curElement)//Pas d'élément courant
                {
                    //ERREUR : CLOSING TAG WHEREAS NONE IS OPEN
                    THROW(XMLParseException_New("Closing tag whereas none is open",
                                            data, lendata, pos - 1));

                }
                if((endData) < pos + curElement->lenTagName)//Impossible que les tags correspondent
                {
                    //ERREUR : TAG MISMATCH
                    THROW(XMLParseException_New("Tag mismatch",
                    data, lendata, pos - 1));
                }
                //Les tags ne correspondent pas
                else if(memcmp(pos, curElement->tagName, curElement->lenTagName) != 0)
                {
                    //ERREUR : TAG MISMATCH
                    THROW(XMLParseException_New("Tag mismatch",
                    data, lendata, pos - 1));
                }
                else//Les tags correspondent, on remonte
                {
                    pos += curElement->lenTagName;
                    curElement = (Element*)(curElement->parent);
                    tag = (char*)memchr(pos, '>', endData - pos);//On cherche la fin du tag
                    pos = tag + 1;//On avance à la fin du tag

                    continue;
                }
            }
            else if(ws == '!')//Ce serait un commentaire ou un CDATA
            {
                if(memcmp(pos, "--", 2) == 0)//C'est bien un commentaire
                {
                    pos += 2;//On va au début du contenu du commentaire
                    tag = (char*)memchrs(pos, endData - pos, "-->", 3);
                    if(!tag)//Commentaire sans fin
                    {
                        //ERREUR : NEVER-ENDING COMMENT
                        THROW(XMLParseException_New("Never-ending comment",
                        data, lendata, pos - 1));
                    }

                    CommentNode *comment = XMLComment_New();
                    XMLTextNode_setEscapedTextContent(comment, pos, tag - pos);
                    APPEND(comment);
                    pos = tag + 3;
                    continue;
                }
                else if(memcmp(pos, "[CDATA[", 7) == 0)//C'est un CDATA
                {
                    pos += 7;//On va au début du contenu du cdata
                    tag = (char*)memchrs(pos, endData - pos, "]]>", 3);
                    if(!tag)//Cdata sans fin
                    {
                        //ERREUR : UNENDED CDATA
                        THROW(XMLParseException_New("Never-ending CDATA",
                        data, lendata, pos - 1));
                    }

                    CDATANode *cdata = XMLCDATA_New();
                    XMLTextNode_setEscapedTextContent(cdata, pos, tag - pos);
                    APPEND(cdata);
                    pos = tag + 3;
                    continue;
                }
                else if(memcmp(pos, "DOCTYPE", 7) == 0)//Doctypes are silently ignored for the moment
                {
                    pos += 7;
                    tag = (char*)memchr(pos, '>', endData - pos);
                    if(!tag)//Doctype sans fin
                    {
                        THROW(XMLParseException_New("Never-ending DOCTYPE",
                        data, lendata, pos - 1));
                    }

                    pos = tag + 1;
                    continue;
                }
                else// ... ?
                {
                    //ERREUR : INVALID TAG
                    THROW(XMLParseException_New("Invalid Tag",
                    data, lendata, pos - 1));
                }
            }
            else if(ws == '?')//Processing Instruction //TODO : add the PI API
            {
                tag = (char*)memchrs(pos, endData - pos, "?>", 2);//Looking for the end of the PI
                if(!tag)//Endless PI
                {
                    THROW(XMLParseException_New("Never-ending Processing instruction",
                    data, lendata, pos - 1));
                }

                pos = tag + 2;
                continue;

            }
            else// ... ?
            {
                //ERREUR : INVALID TAG
                THROW(XMLParseException_New("Invalid Tag",
                data, lendata, pos - 1));
            }
        }//Si tout va bien, on a un nouvel élément
        else
        {
            while(isNameChar(nextUTF8Char(pos, endData - pos)))//On cherche le tagName
            {
                if(pos > endData)
                {
                    //ERREUR : NEVER-ENDING TAG
                    THROW(XMLParseException_New("Never-ending tag",
                    data, lendata, pos - 1));
                }
            }
            pos--;

            Element *elmt = XMLElement_New(tag, pos - tag);
            APPEND(elmt);
            curElement = elmt;
            s = *pos;

            while(pos < endData)//On gère le contenu de l'élément (attributs)
            {
                if(s == '>') break;//Fin de l'élément
                if(s == '/') //Élément auto-fermant
                {
                    pos++;
                    curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                    break;
                }

                if(isNameStartChar(s))//Début d'attribut
                {
                    const char *attrNamestart = pos;
                    while(isNameChar(nextUTF8Char(pos, endData - pos)) && pos < endData){}//On parcourt le nom d'attribut
                    pos--;
                    const char *attrNameEnd = pos;
                    s = *pos;
                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs

                    if(s != '=')
                    {
                        XMLElement_AddAttribute(elmt, attrNamestart, attrNameEnd - attrNamestart);
                        if(s == '>') break;//Fin de l'élément
                        else if (s == '/')//Élément auto-fermant
                        {
                            pos++;
                            curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                            break;
                        }
                        else
                        {
                            //ERREUR : INVALID TAG
                            THROW(XMLParseException_New("Invalid tag",
                            data, lendata, pos - 1));
                        }
                    }

                    pos++; s = *pos;

                    while(isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs

                    char delimiter = s;
                    if(delimiter != '"' && delimiter != '\'')
                    {
                        //ERREUR : EXPECTED ATTRIBUTE DELIMITER
                        THROW(XMLParseException_New("Expected attribute delimiter",
                        data, lendata, pos - 1));
                    }
                    pos++;

                    char* delimiterPos = (char*)memchr(pos, delimiter, endData - pos);

                    XMLElement_AddAttribute(elmt, attrNamestart, attrNameEnd - attrNamestart,
                                       pos, delimiterPos - pos);
                    pos = delimiterPos;

                }

                pos++; s = *pos;
            }

        }
        pos++;
    }

    return elements;

}
