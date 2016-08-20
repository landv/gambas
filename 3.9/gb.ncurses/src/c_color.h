#ifndef __C_COLOR_H
#define __C_COLOR_H

extern GB_DESC CColorDesc[];
extern GB_DESC CColorInfoDesc[];
extern GB_DESC CPairDesc[];

extern void COLOR_init();
extern int CCOLOR_setcolor(short index, float r, float g, float b);
extern int CCOLOR_setcolor_one(short index, float val, int which);
extern short CPAIR_get(short fg, short bg);

#endif /* __C_COLOR_H */
