#include "textnode.h"
#include "CTextNode.h"


/*========== TextNode */

#undef THIS
#define THIS (static_cast<CTextNode*>(_object)->node)

BEGIN_METHOD(CTextNode_new, GB_STRING content)

if(Node::NoInstanciate) return;
if(!THIS)
{
    THIS = new TextNode;
    static_cast<CNode*>(_object)->node = THIS;
}
if(!MISSING(content)) THIS->content = new fwstring(STRING(content));
//else THIS->content = new fwstring();

END_METHOD

BEGIN_METHOD_VOID(CTextNode_free)

delete THIS;

END_METHOD

GB_DESC CTextNodeDesc[] =
{
    GB_DECLARE("XmlTextNode", sizeof(CTextNode)), GB_INHERITS("XmlNode"),
    GB_METHOD("_new", "", CTextNode_new, "[(Content)s]"),
    GB_METHOD("_free", "", CTextNode_free, ""),

    GB_END_DECLARE
};

/*========== CommentNode */

#undef THIS
#define THIS (static_cast<CCommentNode*>(_object)->node)

BEGIN_METHOD_VOID(CCommentNode_new)

if(Node::NoInstanciate) return;
THIS = new CommentNode;
static_cast<CTextNode*>(_object)->node = THIS;
static_cast<CNode*>(_object)->node = THIS;

END_METHOD

BEGIN_METHOD_VOID(CCommentNode_free)

END_METHOD

GB_DESC CCommentNodeDesc[] =
{
    GB_DECLARE("XmlCommentNode", sizeof(CCommentNode)),GB_INHERITS("XmlTextNode"),
    GB_METHOD("_new", "", CCommentNode_new, ""),
    GB_METHOD("_free", "", CCommentNode_free, ""),

    GB_END_DECLARE
};

/*========== CDATANode */

#undef THIS
#define THIS (static_cast<CCDATANode*>(_object)->node)

BEGIN_METHOD_VOID(CCDATANode_new)

THIS = new CDATANode;

static_cast<CTextNode*>(_object)->node = THIS;
static_cast<CNode*>(_object)->node = THIS;

END_METHOD

BEGIN_METHOD_VOID(CCDATANode_free)

END_METHOD

GB_DESC CCDATANodeDesc[] =
{
    GB_DECLARE("XmlCDATANode", sizeof(CCDATANode)),GB_INHERITS("XmlTextNode"),
    GB_METHOD("_new", "", CCDATANode_new, ""),
    GB_METHOD("_free", "", CCDATANode_free, ""),

    GB_END_DECLARE
};
