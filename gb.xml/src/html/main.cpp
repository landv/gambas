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
    Element::ClassName = GB.FindClass("XmlElement");
    TextNode::ClassName = GB.FindClass("XmlTextNode");
    CommentNode::ClassName = GB.FindClass("XmlCommentNode");
    CDATANode::ClassName = GB.FindClass("XmlCDATANode");
    Node::ClassName = GB.FindClass("XmlNode");
    AttrNode::ClassName = GB.FindClass("_XmlAttrNode");
    Document::ClassName = GB.FindClass("XmlDocument");
  return -1;
}

void EXPORT GB_EXIT()
{

}
}
