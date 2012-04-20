#include "CTextNode.h"


/*========== TextNode */

#undef THIS
#define THIS (static_cast<TextNode*>(_object))

BEGIN_METHOD(CTextNode_new, GB_STRING content)

THIS->virt = new TextNode::Virtual(THIS);
if(!MISSING(content)) THIS->content = new wstring(STRING(content));
else THIS->content = new wstring();

END_METHOD

BEGIN_METHOD_VOID(CTextNode_free)

delete THIS->virt;
THIS->virt = 0;
if(THIS->content) delete THIS->content;


END_METHOD

GB_DESC CTextNodeDesc[] =
{
    GB_DECLARE("XmlTextNode", sizeof(TextNode)), GB_INHERITS("XmlNode"),
    GB_METHOD("_new", "", CTextNode_new, "[(Content)s]"),
    GB_METHOD("_free", "", CTextNode_free, ""),

    GB_END_DECLARE
};

/*========== CommentNode */

#undef THIS
#define THIS (static_cast<CommentNode*>(_object))

BEGIN_METHOD_VOID(CCommentNode_new)

delete THIS->virt;
THIS->virt = new CommentNode::Virtual(THIS);

END_METHOD

BEGIN_METHOD_VOID(CCommentNode_free)

END_METHOD

GB_DESC CCommentNodeDesc[] =
{
    GB_DECLARE("XmlCommentNode", sizeof(CommentNode)),GB_INHERITS("XmlTextNode"),
    GB_METHOD("_new", "", CCommentNode_new, ""),
    GB_METHOD("_free", "", CCommentNode_free, ""),

    GB_END_DECLARE
};

/*========== CDATANode */

#undef THIS
#define THIS (static_cast<CDATANode*>(_object))

BEGIN_METHOD_VOID(CCDATANode_new)

delete THIS->virt;
THIS->virt = new CDATANode::Virtual(THIS);

END_METHOD

BEGIN_METHOD_VOID(CCDATANode_free)

END_METHOD

GB_DESC CCDATANodeDesc[] =
{
    GB_DECLARE("XmlCDATANode", sizeof(CDATANode)),GB_INHERITS("XmlTextNode"),
    GB_METHOD("_new", "", CCDATANode_new, ""),
    GB_METHOD("_free", "", CCDATANode_free, ""),

    GB_END_DECLARE
};
