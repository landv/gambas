#ifndef CXMLWRITER_H
#define CXMLWRITER_H

#include "main.h"


#ifndef CLASSES_CPP
extern GB_DESC CWriterDesc[];
#endif

class Writer : public GB_BASE
{
public:
    GB_STREAM *stream;
};

#endif // CXMLWRITER_H 
