#define MAIN_CPP
#ifndef __HMAIN_CPP
#include "main.h"
#include "CDocument.h"
#include "CElement.h"
#include "CNode.h"
#include "CTextNode.h"
#include "CReader.h"
#include "CExplorer.h"
#endif


#ifndef __HMAIN_CPP

GB_INTERFACE GB EXPORT;

extern "C"{
GB_DESC *GB_CLASSES[] EXPORT =
{
  CDocumentDesc, CNodeDesc, CElementAttributesDesc, CElementAttributeNodeDesc, CElementDesc, CTextNodeDesc,
    CCommentNodeDesc, CCDATANodeDesc, CReaderDesc, CReaderNodeDesc, CReaderNodeTypeDesc,
    CReaderNodeAttributesDesc, CReaderReadFlagsDesc, CExplorerDesc, CExplorerReadFlagsDesc, 0
};

int EXPORT GB_INIT(void)
{
   // GBI::InitClasses();
    /*Element::ClassName = GB.FindClass("XmlElement");
    TextNode::ClassName = GB.FindClass("XmlTextNode");
    CommentNode::ClassName = GB.FindClass("XmlCommentNode");
    CDATANode::ClassName = GB.FindClass("XmlCDATANode");
    Node::ClassName = GB.FindClass("XmlNode");
    AttrNode::ClassName = GB.FindClass("_XmlAttrNode");
    Document::ClassName = GB.FindClass("XmlDocument");*/

  return -1;
}

void EXPORT GB_EXIT()
{

}
}
#endif
