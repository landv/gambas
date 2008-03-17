#ifndef __GGAMBASTAG_H
#define __GGAMBASTAG_H

#include "gtag.h"

class gGambasTag: public gTag
{
public:
	gGambasTag(void *p) : gTag(p) { }
	virtual ~gGambasTag() {}
	virtual void ref(void *v) { GB.Ref(v); }
	virtual void unref(void *v) { GB.Unref((void **)&v); }
};

#endif
