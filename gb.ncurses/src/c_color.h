#ifndef __C_COLOR_H
#define __C_COLOR_H

#include <ncurses.h>

#define PAIR_VALID(p)		(p >= 0 && p < COLOR_PAIRS)
#define COLOR_VALID(c)		(c >= -1 && c < COLORS)

#ifndef __C_COLOR_C
extern GB_DESC CColorDesc[];
extern GB_DESC CColorCapabilitiesDesc[];
extern GB_DESC CColorPairDesc[];
extern GB_DESC CColorContentDesc[];
#endif

enum {
	SETPAIR_FORE,
	SETPAIR_BACK
};

void COLOR_init();
int COLOR_setpair(short index, short fg, short bg);
int COLOR_setpair_one(short index, short val, int what);

#endif /* __C_COLOR_H */
