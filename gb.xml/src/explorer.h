#ifndef EXPLORER_H
#define EXPLORER_H

#include "main.h"
#include "document.h"
#include "element.h"
#include "textnode.h"

/* ===== Constantes de retour de Read() ===== */

#define NODE_ELEMENT 1//Retourné au début d'un élément
#define NODE_TEXT 2//Fin d'un nœud texte
#define NODE_COMMENT 3//Fin d'un commentaire
#define NODE_CDATA 4//Fin d'un CDATA
#define READ_END_CUR_ELEMENT 5//Fin d'un élément
#define READ_ERR_EOF 6//Fin du document
#define READ_ATTRIBUTE 7//Lecture d'un attribut
#define FLAGS_COUNT 8


/* Explorer : explore un document déjà chargé en mémoire.*/
class Explorer : public GB_BASE
{
public:
    void Load(Document *doc);


    void Init();//Intitialise le lecteur
    void Clear();//Réinitialise le lecteur
    int MoveNext();//Va au nœud suivant
    int Read();//Continue la lecture du document
    bool flags[FLAGS_COUNT];//Flags de lecture
    bool endElement;//Si on vient de finir la lecture de l'élément courant
    Document *loadedDocument;//Document actuellement chargé
    Node *curNode;
};

#endif // EXPLORER_H
