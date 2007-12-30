

#ifndef __GETOPTIONS_H
#define __GETOPTIONS_H

#include "gambas.h"


#ifndef __GETOPTIONS_C

extern GB_DESC GetOptionsDesc[];

#else

typedef
struct {
  GB_BASE ob;
  char *options;
  char **argv;
  int arg_count;
  char **opt_found;
  char **opt_arg;
  char **invalid;
  GB_ARRAY rest;
  GB_ARRAY cmdline;
  GB_ARRAY return_temp;
  int index;
}
COPTIONS;

#define THIS  OBJECT(COPTIONS)
	
#endif

#endif
