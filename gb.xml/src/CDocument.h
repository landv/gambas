#ifndef CDOCUMENT_H
#define CDOCUMENT_H

#include "../gambas.h"

class Document;

typedef struct CDocument
{
    GB_BASE ob;
    Document *doc;
} CDocument;

#ifndef CLASSES_CPP
extern GB_DESC CDocumentDesc[];
#endif

#endif // CDOCUMENT_H
