/***************************************************************************

  (c) 2012 Adrien Prokopowicz <prokopy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef EXPLORER_H
#define EXPLORER_H

/* ===== Constantes de retour de Read() ===== */

#define NODE_ELEMENT 1//Retourné au début d'un élément
#define NODE_TEXT 2//Fin d'un nœud texte
#define NODE_COMMENT 3//Fin d'un commentaire
#define NODE_CDATA 4//Fin d'un CDATA
#define NODE_ATTRIBUTE 5
#define READ_END_CUR_ELEMENT 6//Fin d'un élément
#define READ_ERR_EOF 7//Fin du document
#define READ_ATTRIBUTE 8//Lecture d'un attribut
#define FLAGS_COUNT 9

class Document;
class Node;


/* Explorer : explore un document déjà chargé en mémoire.*/
class Explorer
{
public:
    Explorer();
    ~Explorer();
    
    void Load(Document *doc);


    void Init();//Intitialise le lecteur
    void Clear();//Réinitialise le lecteur
    int MoveNext();//Va au nœud suivant
    unsigned char Read();//Continue la lecture du document
    bool *flags;//Flags de lecture
    bool endElement;//Si on vient de finir la lecture de l'élément courant
    bool eof;
    Document *loadedDocument;//Document actuellement chargé
    Node *curNode;

    unsigned char state;
};


#endif // EXPLORER_H
