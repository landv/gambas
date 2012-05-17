#define __HMAIN_CPP
#include "main.h"
#include "CElement.h"
#include "CDocument.h"

#include "../main.cpp"
#include "../gbi.cpp"
#include "../utils.cpp"

GB_INTERFACE GB EXPORT;

extern "C"{
GB_DESC *GB_CLASSES[] EXPORT =
{
   CDocumentDesc, CDocumentStyleSheetsDesc, CDocumentScriptsDesc, CElementDesc, 0
};

int EXPORT GB_INIT(void)
{

  return -1;
}

void EXPORT GB_EXIT()
{

}
}
