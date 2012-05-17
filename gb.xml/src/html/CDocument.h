#ifndef HCDOCUMENT_H
#define HCDOCUMENT_H

#include "../CDocument.h"

class HtmlDocument;

typedef struct CHtmlDocument
{
    CDocument d;
    HtmlDocument *doc;
} CHtmlDocument;

#ifndef CLASSES_CPP
extern GB_DESC CDocumentDesc[];
extern GB_DESC CDocumentStyleSheetsDesc[];
extern GB_DESC CDocumentScriptsDesc[];
#endif

#endif
