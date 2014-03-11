#include "htmlparser.h"
#include "htmlelement.h"
#include "../gbinterface.h"
#include <stdlib.h>
#include "../main.h"

/*****    Parser     *****/

void GBparseHTML(const char *data, const size_t lendata, GB_ARRAY *array)
{
    size_t nodeCount;
    size_t i = 0;
    Node **nodes = parseHTML(data, lendata, &nodeCount);
    GB.Array.New(array, GB.FindClass("XmlNode"), nodeCount);

    for(i = 0; i < nodeCount; ++i)
    {
        *(reinterpret_cast<void **>((GB.Array.Get(*array, i)))) =  XML.XMLNode_GetGBObject(nodes[i]);
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
    XML.XMLNode_appendChild(curElement, _elmt); \
}

Node** parseHTML(char const *data, const size_t lendata, size_t *nodeCount)// XML.ThrowXMLParseException)
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
            //XML.Trim(textpos, textlen);
            if(textlen != 0)
            {
                TextNode *text = XML.XMLTextNode_New("", 0);
                XML.XMLTextNode_setEscapedTextContent(text, textpos, textlen);
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
                //XML.Trim(textpos, textlen);
                if(textlen != 0)
                {
                    TextNode *text = XML.XMLTextNode_New("", 0);
                    XML.XMLTextNode_setEscapedTextContent(text, textpos, textlen);
                    APPEND(text);
                }
            }
            break;
        }

        tag++;
        pos = tag;//On avance au caractère trouvé

        //On analyse le contenu du tag
        ws = XML.nextUTF8Char(pos, endData - pos);//On prend le premier caractère

        if(!XML.isNameStartChar(ws))//Ce n'est pas un tagName, il y a quelque chose ...
        {
            if(ws == '/')//C'est un élément de fin
            {
                Element *oldCurElement = curElement;
                while(curElement)
                {
                    if((endData) >= pos + curElement->lenTagName)
                    {
                        if(!(strncasecmp(pos, curElement->tagName, curElement->lenTagName)))
                        {
                            break;
                        }
                    }
                    curElement = (Element*)(curElement->parent);
                }

                if(!curElement)
                {
                    curElement = oldCurElement;
                }
                else
                {
                    pos += curElement->lenTagName;
                    curElement = (Element*)(curElement->parent);
                }

                tag = (char*)memchr(pos, '>', endData - pos);//On cherche la fin du tag
                if(tag) pos = tag + 1;//On avance à la fin du tag

                continue;

            }
            else if(ws == '!')//Ce serait un commentaire ou un CDATA
            {
                if(memcmp(pos, "--", 2) == 0)//C'est bien un commentaire
                {
                    pos += 2;//On va au début du contenu du commentaire
                    tag = (char*)XML.memchrs(pos, endData - pos, "-->", 3);
                    if(!tag)//Commentaire sans fin
                    {
                        //ERREUR : NEVER-ENDING COMMENT
                        XML.ThrowXMLParseException("Never-ending comment",
                        data, lendata, pos - 1);
                    }

                    CommentNode *comment = XML.XMLComment_New("", 0);
                    XML.XMLTextNode_setEscapedTextContent(comment, pos, tag - pos);
                    APPEND(comment);
                    pos = tag + 3;
                    continue;
                }
                else if(memcmp(pos, "[CDATA[", 7) == 0)//C'est un CDATA
                {
                    pos += 7;//On va au début du contenu du cdata
                    tag = (char*)XML.memchrs(pos, endData - pos, "]]>", 3);
                    if(!tag)//Cdata sans fin
                    {
                        //ERREUR : UNENDED CDATA
                        XML.ThrowXMLParseException("Never-ending CDATA",
                        data, lendata, pos - 1);
                    }

                    CDATANode *cdata = XML.XMLCDATA_New("", 0);
                    XML.XMLTextNode_setEscapedTextContent(cdata, pos, tag - pos);
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
                        XML.ThrowXMLParseException("Never-ending DOCTYPE",
                        data, lendata, pos - 1);
                    }

                    pos = tag + 1;
                    continue;
                }
                else// ... ?
                {
                    //ERREUR : INVALID TAG
                    XML.ThrowXMLParseException("Invalid Tag",
                    data, lendata, pos - 1);
                }
            }
            else if(ws == '?')//Processing Instruction //TODO : add the PI API
            {
                tag = (char*)XML.memchrs(pos, endData - pos, "?>", 2);//Looking for the end of the PI
                if(!tag)//Endless PI
                {
                    XML.ThrowXMLParseException("Never-ending Processing instruction",
                    data, lendata, pos - 1);
                }

                pos = tag + 2;
                continue;

            }
            else// ... ?
            {
                //ERREUR : INVALID TAG
                XML.ThrowXMLParseException("Invalid Tag",
                data, lendata, pos - 1);
            }
        }//Si tout va bien, on a un nouvel élément
        else
        {
            while(XML.isNameChar(XML.nextUTF8Char(pos, endData - pos)))//On cherche le tagName
            {
                if(pos > endData)
                {
                    //ERREUR : NEVER-ENDING TAG
                    XML.ThrowXMLParseException("Never-ending tag",
                    data, lendata, pos - 1);
                }
            }
            pos--;

            Element *elmt = XML.XMLElement_New(tag, pos - tag);
            APPEND(elmt);
            curElement = elmt;
            s = *pos;

            while(pos < endData)//On gère le contenu de l'élément (attributs)
            {
                if(s == '>')
                {
                    if(HTMLElement_IsSingle(curElement))
                    {
                        curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                    }
                    else if(XML.GB_MatchString(curElement->tagName, curElement->lenTagName, "script", 6, 1))
                    {
                        pos ++;//On va au début du contenu du script

                        char *scriptTag = (char*)malloc(sizeof(char) * (curElement->lenTagName) + 3);
                        scriptTag[0] = '<'; scriptTag[1] = '/'; scriptTag[curElement->lenTagName + 2] = '>';
                        memcpy(scriptTag + 2, curElement->tagName, curElement->lenTagName);

                        tag = (char*)XML.memchrs(pos, endData - pos, scriptTag, curElement->lenTagName + 3);

                        free(scriptTag);

                        if(!tag)//Script sans fin
                        {
                            XML.ThrowXMLParseException("Never-ending Script",
                            data, lendata, pos - 1);
                        }

                        TextNode *scriptcontent = XML.XMLTextNode_New("", 0);
                        XML.XMLTextNode_setEscapedTextContent(scriptcontent, pos, tag - pos);
                        APPEND(scriptcontent);
                        pos = tag + curElement->lenTagName + 2;
                        curElement = (Element*)(curElement->parent);//Pas d'autres enfants, on remonte
                    }
                    break;//Fin de l'élément
                }
                if(s == '/') //Élément auto-fermant
                {
                    pos++;

                    curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                    break;
                }

                if(XML.isNameStartChar(s))//Début d'attribut
                {
                    const char *attrNamestart = pos;
                    while(XML.isNameChar(XML.nextUTF8Char(pos, endData - pos)) && pos < endData){}//On parcourt le nom d'attribut
                    pos--;
                    const char *attrNameEnd = pos;
                    s = *pos;
                    while(XML.isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs

                    if(s != '=')
                    {
                        XML.XMLElement_AddAttribute(elmt, attrNamestart, attrNameEnd - attrNamestart, "", 0);
                        if(s == '>') break;//Fin de l'élément
                        else if (s == '/')//Élément auto-fermant
                        {
                            pos++;
                            curElement = (Element*)(curElement->parent);//Pas d'enfants, on remonte
                            break;
                        }
                        else
                        {
                            /*//ERREUR : INVALID TAG
                            XML.ThrowXMLParseException("Invalid tag",
                            data, lendata, pos - 1);*/
                            //Tag not ended correctly ? ...
                            pos = (char*)memchr(pos, '>', endData - pos);
                            break;
                        }
                    }

                    pos++; s = *pos;

                    while(XML.isWhiteSpace(s) && pos < endData){pos++; s = *pos;}//On ignore les espaces blancs

                    char delimiter = s;
                    if(delimiter != '"' && delimiter != '\'')
                    {
                        /*//ERREUR : EXPECTED ATTRIBUTE DELIMITER
                        XML.ThrowXMLParseException("Expected attribute delimiter",
                        data, lendata, pos - 1);*/
                        //Seeking to the next white space or tag-end
                        while(!(XML.isWhiteSpace(s)) && pos < endData && s != '>'){pos++; s = *pos;}
                        delimiter = s;
                        char* delimiterPos = (char*)memchr(pos, delimiter, endData - pos);

                        XML.XMLElement_AddAttribute(elmt, attrNamestart, attrNameEnd - attrNamestart,
                                           pos, delimiterPos - pos);
                        pos = delimiterPos;
                        if(delimiter == '>') pos = delimiterPos - 1;
                    }
                    else
                    {
                        pos++;
                        char* delimiterPos = (char*)memchr(pos, delimiter, endData - pos);

                        XML.XMLElement_AddAttribute(elmt, attrNamestart, attrNameEnd - attrNamestart,
                                           pos, delimiterPos - pos);
                        pos = delimiterPos;
                    }





                }

                pos++; s = *pos;
            }

        }
        pos++;

    }

    return elements;

}
