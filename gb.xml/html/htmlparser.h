#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include "htmlmain.h"

void GBparseHTML(char const *data, const size_t lendata, GB_ARRAY *array);
Node** parseHTML(char const *data, const size_t lendata, size_t *nodeCount);// throw(XMLParseException);

#endif // HTMLPARSER_H
