#ifndef CEXPLORER_H
#define CEXPLORER_H

#include "../gambas.h"
#include "../gb_common.h"

class Explorer;

typedef struct CExplorer
{
    GB_BASE ob;
    Explorer *explorer;
} CExplorer;

#ifndef CEXPLORER_CPP
extern GB_DESC CExplorerDesc[];
extern GB_DESC CExplorerReadFlagsDesc[];
#endif

#endif // CEXPLORER_H
