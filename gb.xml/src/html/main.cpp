#define __HMAIN_CPP
#include "main.h"
#include "CElement.h"
#include "CDocument.h"

#include "../main.cpp"

GB_INTERFACE GB EXPORT;

extern "C"{
GB_DESC *GB_CLASSES[] EXPORT =
{
   CDocumentDesc, CDocumentStyleSheetsDesc, CDocumentScriptsDesc, CElementDesc, 0
};

int EXPORT GB_INIT(void)
{
    HtmlDocument::ClassName = GB.FindClass("HtmlDocument");
  return -1;
}

void EXPORT GB_EXIT()
{

}
}
