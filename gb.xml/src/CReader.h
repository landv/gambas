#ifndef CXMLREADER_H
#define CXMLREADER_H

#include "../gambas.h"
#include "../gb_common.h"

class Reader;

typedef struct CReader
{
    GB_BASE ob;
    Reader *reader;
} CReader;


#ifndef CLASSES_CPP
extern GB_DESC CReaderDesc[];
extern GB_DESC CReaderNodeDesc[];
extern GB_DESC CReaderNodeTypeDesc[];
extern GB_DESC CReaderNodeAttributesDesc[];
extern GB_DESC CReaderReadFlagsDesc[];
#endif


#endif // CXMLREADER_H
