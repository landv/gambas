#ifndef READER_H
#define READER_H

#include "document.h"
#include "element.h"
#include "textnode.h"

#define NODE_ELEMENT 1
#define NODE_TEXT 2
#define NODE_COMMENT 3
#define NODE_CDATA 4
#define READ_END_CUR_ELEMENT 5
#define READ_ERR_EOF 6
#define READ_ATTRIBUTE 7
#define FLAGS_COUNT 8

//Niveau de lecture des tags spéciaux (commentaires, CDATA, prologue ...)
//Commentaire <!-- -->
#define COMMENT_TAG_STARTCHAR_1 1 //Caractère !
#define COMMENT_TAG_STARTCHAR_2 2 //Caractère -
#define COMMENT_TAG_STARTCHAR_3 3 //Caractère - (2e)
#define COMMENT_TAG_ENDCHAR_1 4 //Caractère -
#define COMMENT_TAG_ENDCHAR_2 5 //Caractère - (2e)

//Prologue <? ?>
#define PROLOG_TAG_ENDCHAR 6//Caractère ?


class Reader : public GB_BASE
{
public :

    Document *storedDocument;//Document stocké
    Node *curNode;//Nœud en cours de lecture
    Node *foundNode;//Noœud dont on vient de finir la lecture
    Element *curElmt;//Nœud en cours de lecture
    bool keepMemory;//Si on doit garder toute l'arborescence en mémoire
    int pos;//Position dans le flux
    bool inTag;//Si on est en train de lire un tag incomplet
    bool inAttr;//Si on est en train de lire un attribut
    bool inAttrName;//Si on est en train de lire un nom d'attribut
    bool inAttrVal;//Si on est en train de lire une valeur d'attribut
    bool inEndTag;//Si on est dans un tag de fin
    bool inNewTag;//Si on est dans un nouveau tag
    bool inCommentTag;//Si on est dans un tag commentaire
    bool inXMLProlog;//Si on est dans un prologue xml
    unsigned char specialTagLevel;//Niveau de lecture d'un tag spécial
    int depth;//Profondeur du nœud courant
    wstring *attrName;//Nom de l'attribut en cours de lecture
    wstring *attrVal;//Valeur de l'attribut en cours de lecture
    wstring *content;//Contenu du nœud texte en cours de lecture
    
    vector<Node*> *storedElements;
    
    void InitReader();//Intitialise le lecteur
    int ReadChar(wstring car);//Lit un caractère
    void ClearReader();//Réinitialise le lecteur
    bool flags[FLAGS_COUNT];//Flags de lecture
};


#endif
