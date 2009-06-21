#ifndef __GSHARE_H
#define __GSHARE_H

#include "gtag.h"

//#define DEBUG_ME

#ifdef DEBUG_ME

class gShare
{
public:
  gShare() { nref = 1; tref = 1; tag = NULL; fprintf(stderr, "gShare: %p (1)\n", this); }
  
	virtual ~gShare() 
	{ 
		fprintf(stderr, "~gShare: %p tag = %p\n", this, tag); 
		if (tag) 
		{
			while (nref > tref) 
			{
				nref--;
				tag->unref(); 
			}
		}
	}
  
  void ref() { nref++; if (tag) tag->ref(); fprintf(stderr, "gShare::ref: %p (%d)\n", this, nref); }
  void unref() { nref--; fprintf(stderr, "gShare::unref: %p (%d)\n", this, nref); if (nref <= 0) delete this; else if (tag) tag->unref(); }
  int refCount() { return nref; }
  
  void setTag(gTag *t) { fprintf(stderr, "gShare::setTag: %p (%d)\n", this, nref); tag = t; tref = nref; for (int i = 0; i < (nref - 1); i++) tag->ref(); }
  gTag *getTag() { return tag; }
  void *getTagValue() { return tag->get(); }
  
protected:
  static void assign(gShare **dst, gShare *src = 0)
  {
    if (src) src->ref();
    
		if (*dst) 
		{
			gShare *tmp = *dst;
			*dst = 0;
			tmp->unref();
		}
		
    *dst = src;
  }
  
private:
  int nref;
	int tref;
  gTag *tag;
};

#else

class gShare
{
public:
  gShare() { nref = 1; tag = NULL; }
  virtual ~gShare() { if (tag) { while (nref > 1) nref--, tag->unref(); delete tag; } }
  
  void ref() { nref++; if (tag) tag->ref(); }
  void unref() { nref--; if (nref <= 0) delete this; else if (tag) tag->unref(); }
  int refCount() { return nref; }
  
  void setTag(gTag *t) { tag = t; for (int i = 0; i < (nref - 1); i++) tag->ref(); }
  gTag *getTag() { return tag; }
  void *getTagValue() { return tag->get(); }
  
protected:
  static void assign(gShare **dst, gShare *src = 0)
  {
    if (src) src->ref();
    if (*dst) (*dst)->unref();
    *dst = src;
  }
  
private:
  int nref;
	//int tref;
  gTag *tag;
};

#endif

#endif

