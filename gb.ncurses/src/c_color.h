#ifndef __C_COLOR_H
#define __C_COLOR_H

#include <ncurses.h>

#define PAIR_VALID(p)		(p >= 0 && p < COLOR_PAIRS)
#define COLOR_VALID(c)		(c >= -1 && c < COLORS)

#ifndef __C_COLOR_C
extern GB_DESC CColorDesc[];
extern GB_DESC CColorInfoDesc[];
extern GB_DESC CPairDesc[];
extern GB_DESC CPairInfoDesc[];
#endif

enum {
	SETPAIR_FORE,
	SETPAIR_BACK
};

void COLOR_init();
int COLOR_setcolor(short, float, float, float);
int COLOR_setcolor_one(short, float, int);
int COLOR_setpair(short, short, short);
int COLOR_setpair_one(short, short, int);

#endif /* __C_COLOR_H */
