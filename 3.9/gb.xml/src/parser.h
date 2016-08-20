#ifndef PARSER_H
#define PARSER_H

#include "main.h"
#include "utils.h"

void GBparse(const char *data, const size_t lendata, GB_ARRAY *array, DocumentType docType);
Node** parse(char const *data, const size_t lendata, size_t *nodeCount, DocumentType docType) throw(XMLParseException);

void GBparseXML(char const *data, const size_t lendata, GB_ARRAY *array);
Node** parseXML(char const *data, const size_t lendata, size_t *nodeCount) throw(XMLParseException);


#endif // PARSER_H
